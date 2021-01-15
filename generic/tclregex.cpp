#include <string>
#include <re2/re2.h>

#define NS "regex"

/*
 * For C++ compilers, use extern "C"
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <tcl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Only the _Init function is exported.
 */

extern DLLEXPORT int    Regex_rez_Init(Tcl_Interp * interp);

/*
 * end block for C++
 */

#ifdef __cplusplus
}
#endif

typedef struct ThreadSpecificData {
  int initialized;                /* initialization flag */
  Tcl_HashTable *re2_hashtblPtr; /* per thread hash table. */
  int re2_count;
} ThreadSpecificData;

static Tcl_ThreadDataKey dataKey;

TCL_DECLARE_MUTEX(myMutex);


void RE2_Thread_Exit(ClientData clientdata)
{
  ThreadSpecificData *tsdPtr = (ThreadSpecificData *)
      Tcl_GetThreadData(&dataKey, sizeof(ThreadSpecificData));

  if(tsdPtr->re2_hashtblPtr) {
    Tcl_DeleteHashTable(tsdPtr->re2_hashtblPtr);
    ckfree(tsdPtr->re2_hashtblPtr);
  }
}


static int RE2_FUNCTION(void *cd, Tcl_Interp *interp, int objc,Tcl_Obj *const*objv){
  int choice;
  re2::RE2* re;
  Tcl_HashEntry *hashEntryPtr;
  char *re2Handle;

  ThreadSpecificData *tsdPtr = (ThreadSpecificData *)
      Tcl_GetThreadData(&dataKey, sizeof(ThreadSpecificData));

  if (tsdPtr->initialized == 0) {
    tsdPtr->initialized = 1;
    tsdPtr->re2_hashtblPtr = (Tcl_HashTable *) ckalloc(sizeof(Tcl_HashTable));
    Tcl_InitHashTable(tsdPtr->re2_hashtblPtr, TCL_STRING_KEYS);
  }

  static const char *FUNC_strs[] = {
    "fullmatch",
    "partialmatch",
    "replace",
    "globalreplace",
    "close",
    0
  };

  enum FUNC_enum {
    FUNC_FULLMATCH,
    FUNC_PARTIALMATCH,
    FUNC_REPLACE,
    FUNC_GLOBALREPLACE,
    FUNC_CLOSE,
  };

  if( objc < 2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "SUBCOMMAND ...");
    return TCL_ERROR;
  }

  if( Tcl_GetIndexFromObj(interp, objv[1], FUNC_strs, "option", 0, &choice) ){
    return TCL_ERROR;
  }

  /*
   * Get the re2::RE2 value
   */
  re2Handle = Tcl_GetStringFromObj(objv[0], 0);
  hashEntryPtr = Tcl_FindHashEntry( tsdPtr->re2_hashtblPtr, re2Handle );
  if( !hashEntryPtr ) {
    if( interp ) {
        Tcl_Obj *resultObj = Tcl_GetObjResult( interp );

        Tcl_AppendStringsToObj( resultObj, "invalid re2 handle ", re2Handle, (char *)NULL );
    }

    return TCL_ERROR;
  }

  re = (re2::RE2 *)(uintptr_t)Tcl_GetHashValue( hashEntryPtr );

  switch( (enum FUNC_enum)choice ){

    case FUNC_FULLMATCH: {
      const char *match_string = NULL;
      re2::StringPiece input;
      int len = 0;
      Tcl_Obj *pResultStr = NULL;

      if( objc != 3 ){
        Tcl_WrongNumArgs(interp, 2, objv, "text");
        return TCL_ERROR;
      }

      match_string = Tcl_GetStringFromObj(objv[2], &len);
      if(!match_string || len < 1) {
          if( interp ) {
            Tcl_Obj *resultObj = Tcl_GetObjResult( interp );
            Tcl_AppendStringsToObj( resultObj, "Text is empty or null", (char *)NULL );
          }

          return TCL_ERROR;
      }

      input = re2::StringPiece(match_string, len);

      if(re2::RE2::FullMatch(input, *re)) {
          pResultStr = Tcl_NewBooleanObj( 1 );
      } else {
          pResultStr = Tcl_NewBooleanObj( 0 );
      }

      Tcl_SetObjResult(interp, pResultStr);
      break;
    }

    case FUNC_PARTIALMATCH: {
      const char *match_string = NULL;
      re2::StringPiece input;
      int len = 0;
      Tcl_Obj *pResultStr = NULL;

      if( objc != 3 ){
        Tcl_WrongNumArgs(interp, 2, objv, "text");
        return TCL_ERROR;
      }

      match_string = Tcl_GetStringFromObj(objv[2], &len);
      if(!match_string || len < 1) {
          if( interp ) {
            Tcl_Obj *resultObj = Tcl_GetObjResult( interp );
            Tcl_AppendStringsToObj( resultObj, "Text is empty or null", (char *)NULL );
          }

          return TCL_ERROR;
      }

      input = re2::StringPiece(match_string, len);

      if(re2::RE2::PartialMatch(input, *re)) {
          pResultStr = Tcl_NewBooleanObj( 1 );
      } else {
          pResultStr = Tcl_NewBooleanObj( 0 );
      }

      Tcl_SetObjResult(interp, pResultStr);
      break;
    }

    case FUNC_REPLACE: {
      const char *match_string = NULL;
      const char *match_rewrite = NULL;
      re2::StringPiece input;
      int len = 0, len2 = 0;
      std::string result;
      Tcl_Obj *pResultStr = NULL;

      if( objc != 4 ){
        Tcl_WrongNumArgs(interp, 2, objv, "text rewrite ");
        return TCL_ERROR;
      }

      match_string = Tcl_GetStringFromObj(objv[2], &len);
      if(!match_string || len < 1) {
          if( interp ) {
            Tcl_Obj *resultObj = Tcl_GetObjResult( interp );
            Tcl_AppendStringsToObj( resultObj, "Text is empty or null", (char *)NULL );
          }

          return TCL_ERROR;
      }

      match_rewrite = Tcl_GetStringFromObj(objv[3], &len2);
      if(!match_rewrite || len2 < 1) {
          if( interp ) {
            Tcl_Obj *resultObj = Tcl_GetObjResult( interp );
            Tcl_AppendStringsToObj( resultObj, "Rewrite string is empty or null", (char *)NULL );
          }

          return TCL_ERROR;
      }

      result = std::string(match_string);
      input = re2::StringPiece(match_rewrite, len2);

      if(re2::RE2::Replace(&result, *re, input)) {
          pResultStr = Tcl_NewStringObj( result.c_str(), -1 );
      } else {
          return TCL_ERROR;
      }

      Tcl_SetObjResult(interp, pResultStr);
      break;
    }

    case FUNC_GLOBALREPLACE: {
      const char *match_string = NULL;
      const char *match_rewrite = NULL;
      re2::StringPiece input;
      int len = 0, len2 = 0;
      std::string result;
      Tcl_Obj *pResultStr = NULL;

      if( objc != 4 ){
        Tcl_WrongNumArgs(interp, 2, objv, "text rewrite ");
        return TCL_ERROR;
      }

      match_string = Tcl_GetStringFromObj(objv[2], &len);
      if(!match_string || len < 1) {
          if( interp ) {
            Tcl_Obj *resultObj = Tcl_GetObjResult( interp );
            Tcl_AppendStringsToObj( resultObj, "Text is empty or null", (char *)NULL );
          }

          return TCL_ERROR;
      }

      match_rewrite = Tcl_GetStringFromObj(objv[3], &len2);
      if(!match_rewrite || len2 < 1) {
          if( interp ) {
            Tcl_Obj *resultObj = Tcl_GetObjResult( interp );
            Tcl_AppendStringsToObj( resultObj, "Rewrite string is empty or null", (char *)NULL );
          }

          return TCL_ERROR;
      }

      result = std::string(match_string);
      input = re2::StringPiece(match_rewrite, len2);

      if(re2::RE2::GlobalReplace(&result, *re, input)) {
          pResultStr = Tcl_NewStringObj( result.c_str(), -1 );
      } else {
          return TCL_ERROR;
      }

      Tcl_SetObjResult(interp, pResultStr);
      break;
    }

    case FUNC_CLOSE: {
      if( objc != 2 ){
        Tcl_WrongNumArgs(interp, 2, objv, 0);
        return TCL_ERROR;
      }

      delete re;

      Tcl_MutexLock(&myMutex);
      if( hashEntryPtr )  Tcl_DeleteHashEntry(hashEntryPtr);
      Tcl_MutexUnlock(&myMutex);

      Tcl_DeleteCommand(interp, re2Handle);
      Tcl_SetObjResult(interp, Tcl_NewIntObj( 0 ));

      break;
    }
  }

  return TCL_OK;
}


static int REGEX_MAIN(void *cd, Tcl_Interp *interp, int objc,Tcl_Obj *const*objv){
  int choice;

  ThreadSpecificData *tsdPtr = (ThreadSpecificData *)
      Tcl_GetThreadData(&dataKey, sizeof(ThreadSpecificData));

  if (tsdPtr->initialized == 0) {
    tsdPtr->initialized = 1;
    tsdPtr->re2_hashtblPtr = (Tcl_HashTable *) ckalloc(sizeof(Tcl_HashTable));
    Tcl_InitHashTable(tsdPtr->re2_hashtblPtr, TCL_STRING_KEYS);
  }

  static const char *REG_strs[] = {
    "create",
    0
  };

  enum REG_enum {
    DB_CREATE,
  };

  if( objc < 2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "SUBCOMMAND ...");
    return TCL_ERROR;
  }

  if( Tcl_GetIndexFromObj(interp, objv[1], REG_strs, "option", 0, &choice) ){
    return TCL_ERROR;
  }

  switch( (enum REG_enum)choice ){

    case DB_CREATE: {
      char *zArg;
      re2::RE2* re;
      re2::RE2::Options options;
      Tcl_HashEntry *newHashEntryPtr;
      int len;
      char handleName[16 + TCL_INTEGER_SPACE];
      Tcl_Obj *pResultStr = NULL;
      int newvalue;
      int i = 0;
      const char *regexp = NULL;

      if( objc < 3 || (objc&1)!=1 ){
          Tcl_WrongNumArgs(interp, 2, objv,
          "expr_string ?-utf8 boolean? ?-posix boolean? ?-longest-match boolean? ?-nocase boolean? "
          );

        return TCL_ERROR;
      }

      regexp = Tcl_GetStringFromObj(objv[2], &len);
      if(!regexp || len < 1) {
          if( interp ) {
            Tcl_Obj *resultObj = Tcl_GetObjResult( interp );
            Tcl_AppendStringsToObj( resultObj, "No regular expression!!!", (char *)NULL );
          }

          return TCL_ERROR;
      }

      for(i=3; i+1<objc; i+=2){
        zArg = Tcl_GetStringFromObj(objv[i], 0);

        if( strcmp(zArg, "-utf8")==0 ){
            int b;
            if( Tcl_GetBooleanFromObj(interp, objv[i+1], &b) ) return TCL_ERROR;
            if( b ){
              options.set_encoding(RE2::Options::EncodingUTF8);
            }else{
              options.set_encoding(RE2::Options::EncodingLatin1);
            }
        } else if( strcmp(zArg, "-posix")==0 ){
            int b;
            if( Tcl_GetBooleanFromObj(interp, objv[i+1], &b) ) return TCL_ERROR;
            if( b ){
              options.set_posix_syntax(true);
            }else{
              options.set_posix_syntax(false);
            }
        } else if( strcmp(zArg, "-longest-match")==0 ){
            int b;
            if( Tcl_GetBooleanFromObj(interp, objv[i+1], &b) ) return TCL_ERROR;
            if( b ){
              options.set_longest_match(false);
            }else{
              options.set_longest_match(true);
            }
        } else if( strcmp(zArg, "-nocase")==0 ){
            int b;
            if( Tcl_GetBooleanFromObj(interp, objv[i+1], &b) ) return TCL_ERROR;
            if( b ){
              options.set_case_sensitive(false);
            }else{
              options.set_case_sensitive(true);
            }
        } else{
           Tcl_AppendResult(interp, "unknown option: ", zArg, (char*)0);
           return TCL_ERROR;
        }
      }

      re = new re2::RE2(regexp, options);

      if(!re->ok()) {
          if( interp ) {
            Tcl_Obj *resultObj = Tcl_GetObjResult( interp );
            Tcl_AppendStringsToObj( resultObj, "ERROR: create failed", (char *)NULL );
          }

          return TCL_ERROR;
      }

      Tcl_MutexLock(&myMutex);
      sprintf( handleName, "tclregex-re2%d", tsdPtr->re2_count++ );

      pResultStr = Tcl_NewStringObj( handleName, -1 );

      newHashEntryPtr = Tcl_CreateHashEntry(tsdPtr->re2_hashtblPtr, handleName, &newvalue);
      Tcl_SetHashValue(newHashEntryPtr, (ClientData)(uintptr_t) re);
      Tcl_MutexUnlock(&myMutex);

      Tcl_CreateObjCommand(interp, handleName, (Tcl_ObjCmdProc *) RE2_FUNCTION,
          (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);

      Tcl_SetObjResult(interp, pResultStr);

      break;
    }
  }

  return TCL_OK;
}


/*
 *----------------------------------------------------------------------
 *
 * Regex_rez_Init --
 *
 *	Initialize the new package.
 *
 * Results:
 *	A standard Tcl result
 *
 * Side effects:
 *	The Regex_rez_Init package is created.
 *
 *----------------------------------------------------------------------
 */

int
Regex_rez_Init(Tcl_Interp *interp)
{
    Tcl_Namespace *nsPtr; /* pointer to hold our own new namespace */

    if (Tcl_InitStubs(interp, "8.5", 0) == NULL) {
	return TCL_ERROR;
    }

    if (Tcl_PkgProvide(interp, PACKAGE_NAME, PACKAGE_VERSION) != TCL_OK) {
	return TCL_ERROR;
    }

    /*
     * Create the namespace named re2.
     * re2 command is too short, we need a namespace.
     */
    nsPtr = Tcl_CreateNamespace(interp, NS, NULL, NULL);
    if (nsPtr == NULL) {
        return TCL_ERROR;
    }

    /*
     *  Tcl_GetThreadData handles the auto-initialization of all data in
     *  the ThreadSpecificData to NULL at first time.
     */
    Tcl_MutexLock(&myMutex);
    ThreadSpecificData *tsdPtr = (ThreadSpecificData *)
        Tcl_GetThreadData(&dataKey, sizeof(ThreadSpecificData));

    if (tsdPtr->initialized == 0) {
        tsdPtr->initialized = 1;
        tsdPtr->re2_hashtblPtr = (Tcl_HashTable *) ckalloc(sizeof(Tcl_HashTable));
        Tcl_InitHashTable(tsdPtr->re2_hashtblPtr, TCL_STRING_KEYS);

        tsdPtr->re2_count = 0;
    }
    Tcl_MutexUnlock(&myMutex);

    /* Add a thread exit handler to delete hash table */
    Tcl_CreateThreadExitHandler(RE2_Thread_Exit, (ClientData)NULL);

    Tcl_CreateObjCommand(interp, NS "::re2", (Tcl_ObjCmdProc *) REGEX_MAIN,
        (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);

    return TCL_OK;
}

