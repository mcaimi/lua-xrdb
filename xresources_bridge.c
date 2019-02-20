/*
   XResources LUA interface functions
   - Bridge library to get/set X resources from LUA.
   v0.1
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Lua API includes
#include <lua.h>

// defines
#include "xresources_bridge.h"

// type lookup table
lookuptable type_handlers[] = {
  { "Integer", TINTEGER }, 
  { "Float", TFLOAT },
  { "String", TSTRING }
};

static int handlers_size = sizeof(type_handlers)/sizeof(type_handlers[0]);
// correctly handle type lookup in xrdb database
int query_type(const char *type) {
  for (int i=0; i < handlers_size; i++) {
    // compare and act accordingly
    if (strncmp(type_handlers[i].name, type, strlen(type)) == 0) {
      return type_handlers[i].index;
    }
  }
  // type not found
#ifdef __DEBUG
    printf("LUA-XRDB: query_type() -> Type=%s not found in lookup table\n", type);
#endif
  return TFAIL;
}

// connect to the undelying Xorg server and get a connection
// handler object.
XrmDatabase open_resource_database(Display *dpy) {
  // database holder object
  XrmDatabase db = NULL;
  // resource manager pointer (string content)
  char *resource_manager;
  
  // connect to the resource manager and get a pointer to the database
  resource_manager = XResourceManagerString(dpy);
#ifdef __DEBUG
  printf("LUA-XRDB: contents of resource_manager %s\n", resource_manager);
#endif
  
  // get database
  db = XrmGetStringDatabase(resource_manager);

  // ok return pointer to XrmDatabase
#ifdef __DEBUG
  printf("LUA-XRDB: Opened XrmDatabase Connection -> XrmDatabase *db=0x%p\n", (void *)&db);
#endif
  return (XrmDatabase)(db);
}

// initialize X11 connection
int open_connection(char *display_name, Display **dpy) {
  *dpy = NULL;

  // open display connection
  // If NULL, grab the value of $DISPLAY
  *dpy = XOpenDisplay(display_name);
  
  // initialize Xrm library
#ifdef __DEBUG
    printf("LUA-XRDB: Initializing Xrm -> Display *dpy=0x%p\n", (void *)*dpy);
#endif
  XrmInitialize();
  
  // return display object
  if (*dpy == NULL)
    return -1;
  return 0;
}

// close a live X11 connection
int close_display(Display *dpy) {
  // close connection if connection handler is live
  if ISNOTNULL(dpy) {
#ifdef __DEBUG
    printf("LUA-XRDB: Closing Connection to X11 -> Display *dpy=0x%p\n", (void *)dpy);
#endif
    XCloseDisplay(dpy);
    return ECONDITIONOK;
  }

  // return 1, connection is already closed or no connection at all
  // has been previously made
  return ECONDITIONFAIL;
}
// destroy xrm database allocated resources
void free_xrdb(XrmDatabase db) {
#ifdef __DEBUG
    printf("LUA-XRDB: Freeing resource database  -> *db=0x%p\n", (void *)&db);
#endif
  XrmDestroyDatabase(db);
}

// load a resource from database
int resource_load(XrmDatabase db, char *resource_name, char *resource_class, enum resource_type type, void *dest) {
  // return pointers: STRING
  char **str_value = (char**)dest;
  // INTEGER
  int *int_value = (int *)dest;
  // FLOAT
  float *float_value = (float *)dest;

  // interface variables
  char *retrieved_type = NULL;
  char rname[256], rclass[256];
  XrmValue return_value;
  
  // initialize and format input strings
  memset(rname, 0, sizeof(rname));
  memset(rclass, 0, sizeof(rname));
  snprintf(rname, sizeof(rname), "%s", resource_name);
  snprintf(rclass, sizeof(rclass), "%s", (resource_class != NULL) ? resource_class : resource_name);
  rname[sizeof(rname) - 1] = rclass[sizeof(rclass) - 1] = '\0';

#ifdef __DEBUG
  printf("RESOURCE NAME: %s, RESOURCE CLASS: %s\n", rname, rclass);
#endif

  // call X to get the resource
  XrmGetResource(db, rname, rclass, &retrieved_type, &return_value);
#ifdef __DEBUG
  printf("RESOURCE TYPE: %s\n", retrieved_type);
#endif

  if (return_value.addr == NULL) { return 1; }

  // parse value
  switch(type) {
    case TSTRING:
      *str_value = return_value.addr;
#ifdef __DEBUG
      printf("STRING VALUE: %s\n", (char *)*str_value);
#endif
      break;
    case TINTEGER:
      *int_value = strtoul(return_value.addr, NULL, 10);
#ifdef __DEBUG
      printf("INTEGER VALUE: %d\n", *((int *)dest));
#endif
      break;
    case TFLOAT:
      *float_value = strtof(return_value.addr, NULL);
#ifdef __DEBUG
      printf("FLOAT VALUE %f\n", *((float *)dest));
#endif
      break;
    default:
#ifdef __DEBUG
      printf("FAILURE: unknown resource type %d\n", type);
#endif
      break;
  }

#ifdef __DEBUG
  printf("returning from resource_load()\n");
#endif
  return 0;
}

// Module functions
static int xrdb_get_resource(lua_State *L) {
  Display *dpy = NULL;
  bool found = 0;

  // Xresources handler
  XrmDatabase xrm_database = NULL;
  
  // init connection to xrdb
  if (open_connection(NULL, &dpy) < 0) {
    return -1;
  }
#ifdef __DEBUG
    printf("LUA-XRDB: Display *dpy=0x%p\n", (void *)dpy);
#endif

  xrm_database = open_resource_database(dpy);
  // retrieve resource value from database
  char *xresource_name = NULL;
  char *xresource_type = NULL;

  // values
  char *rstring = NULL;
  int rint = 0;
  float rfloat = 0;

  // get the wanted rescurce name from the lua stack
  if ((!lua_isnoneornil(L, -1) && lua_isstring(L, -1)) && (!lua_isnoneornil(L, -2) && lua_isstring(L, -2))) {
    xresource_name = (char *)lua_tostring(L, -2);
    xresource_type = (char *)lua_tostring(L, -1);
#ifdef __DEBUG
    printf("LUA-XRDB(xrdb_get_resource): Value From Lua C Stack: Want to read (resource_name) = %s of type '%s'\n", xresource_name, xresource_type);
#endif

    switch(query_type(xresource_type)) {
    case TSTRING:
      if (resource_load(xrm_database, xresource_name, NULL, query_type(xresource_type), (void *)&rstring) == 0) {
        if ISNOTNULL((char*) rstring) {
#ifdef __DEBUG
          printf("Pushing string value: \"%s\" of len %ld bytes to lua stack\n", (char*)rstring, strlen(rstring));
#endif
          lua_pushlstring(L, rstring, strlen(rstring));
          found ^= 1;
        }
#ifdef __DEBUG
        else {
          printf("*rstring is NULL\n");
        }
#endif
      }
      break;
    case TINTEGER:
      if (resource_load(xrm_database, xresource_name, NULL, query_type(xresource_type), (void *)&rint) == 0) {

#ifdef __DEBUG
      printf("Pushing integer value: %d to lua stack\n", rint);
#endif
        lua_pushnumber(L, (double)(rint));
        found ^= 1;
      }
      break;
    case TFLOAT:
      if (resource_load(xrm_database, xresource_name, NULL, query_type(xresource_type), (void *)&rfloat) == 0) {
      
#ifdef __DEBUG
        printf("Pushing float value: %f to lua stack\n", rfloat);
#endif
        lua_pushnumber(L, (double)(rfloat));
        found ^= 1;
      }
      break;
    default:
      break;
    }
  }

  // free the Xrdb instance
  free_xrdb(xrm_database);

  // close connection to X server
  close_display(dpy);

  // no resource found
  return found;
}

// insert new resource into database
static int xrdb_set_resource(lua_State *L) {
  Display *dpy = NULL;  
  Window root_win;

  // Xresources handler
  XrmDatabase xrm_database = NULL;
  
  // init connection to xrdb
  if (open_connection(NULL, &dpy) < 0) {
    return -1;
  }
#ifdef __DEBUG
    printf("LUA-XRDB (set resource): Display *dpy=0x%p\n", (void *)dpy);
#endif

  xrm_database = open_resource_database(dpy);
 
  // placeholders for values passed on the Lua interpreter stack
  char *xresource_name = NULL;
  char *xresource_value = NULL;
  
  // for now, treat values as strings.
  const char *xresource_type = "String";

  XrmValue resource_value;
  resource_value.size = 0;
  resource_value.addr = NULL;
  char *resource_string = NULL;
  root_win = DefaultRootWindow(dpy);

  // check and retrieve name/value pair from the lua stack
  if ((!lua_isnoneornil(L, -1) && lua_isstring(L, -1)) && (!lua_isnoneornil(L, -2) && lua_isstring(L, -2))) {
    xresource_name = (char *)lua_tostring(L, -2);
    xresource_value = (char *)lua_tostring(L, -1);

#ifdef __DEBUG
    printf("LUA-XRDB(xrdb_set_resource): Values From Lua C Stack: Want to set (resource_name) = %s with value '%s' of type '%s'\n", xresource_name, xresource_value, xresource_type);
#endif

    // set new resource
    resource_string = (char *)malloc(sizeof(char) * strlen(xresource_name) + strlen(xresource_value) + 10); //buffer
    memset(resource_string, 0, strlen(resource_string));
    // build string resource line
    snprintf(resource_string, strlen(resource_string) - 1, "%s: %s\n", xresource_name, xresource_value);

    // build XrmValue for this resource..
    resource_value.addr = (char *)malloc(sizeof(char) * strlen(xresource_value) + 1);
    memset(resource_value.addr, 0, strlen(xresource_value) + 1);
    memcpy(resource_value.addr, xresource_value, strlen(xresource_value));
    resource_value.size = strlen(xresource_value);

#ifdef __DEBUG
    printf("LUA-XRDB(xrdb_set_resource): VALUE (address) %s (0x%p), type %s\n", resource_value.addr, resource_value.addr, xresource_type);
#endif
    // insert XrmValue in local database
    XrmPutResource(&xrm_database, xresource_name, xresource_type, &resource_value);
    // assign database to display
    XrmSetDatabase(dpy, xrm_database);

    // append property to the resource manager (global)
    XChangeProperty(dpy, 
                    root_win, 
                    XA_RESOURCE_MANAGER, 
                    XA_STRING, 
                    8, 
                    PropModeAppend, 
                    (const unsigned char *)resource_string, 
                    strlen(resource_string));
#ifdef __DEBUG
    printf("LUA-XRDB(xrdb_set_resource): Dumping db to /tmp/database.dump\n"); 
    XrmPutFileDatabase(xrm_database, "/tmp/database.dump");
#endif
  }

  // cleanup resources.
  if (resource_value.addr != NULL)
#ifdef __DEBUG
    printf("LUA-XRDB(xrdb_set_resource): Freeing XrmValue addr field\n");
#endif
    free(resource_value.addr);

  // free the Xrdb instance
  free_xrdb(xrm_database);
  // close connection to X server
  close_display(dpy);

  // return status
  return 0;
}

/*
 *  module initialization function
 *  This one maps lua function names to C function names
 */
int luaopen_xresources(lua_State *L) {
  // Xresources get
  lua_register(L, "xrdb_get_resource", xrdb_get_resource);
  // Xresources set
  lua_register(L, "xrdb_set_resource", xrdb_set_resource);
  // ok module initialized
  return 0;
}
