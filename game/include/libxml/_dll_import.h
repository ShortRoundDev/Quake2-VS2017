#ifndef __DLL_IMPORT_H__
#define __DLL_IMPORT_H__ 1
						  
#ifndef __GNUC__
# define __DLL_IMPORT  __declspec(dllimport)
# define __DECLSPEC_SUPPORTED
#else /* __GNUC__ */
# ifdef __declspec
   /* note the extern at the end. This is needed to work around GCC's
      limitations in handling dllimport attribute.  */
#  define __DLL_IMPORT __attribute__((dllimport)) extern
#  define __DECLSPEC_SUPPORTED
# else
#  undef __DECLSPEC_SUPPORTED
#  undef __DLL_IMPORT
# endif 
#endif /* __GNUC__ */

#endif /* __DLL_IMPORT_H__ */

