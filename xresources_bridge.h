/*
 * X Resources LUA interface
 * v0.1
 */

// X.org libraries includes
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xproto.h>

// error definitions
#define ECONDITIONOK     0
#define ECONDITIONFAIL  -1
#define EQUERYFAILED    -2
#define EFREEFAILED     -3
#define ECONNISNULL     NULL

// macros
#define ISNULL(variable)  (variable == NULL)
#define ISNOTNULL(variable)  (variable != NULL)

// structs and variables
struct __xtypes_lookup {
  char *name;
  int index;
};
typedef struct __xtypes_lookup lookuptable;
typedef unsigned int bool;

enum resource_type {
  TSTRING = 0,
  TINTEGER = 1,
  TFLOAT = 2,
  TFAIL = -1
};

