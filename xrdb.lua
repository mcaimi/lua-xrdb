--
-- XRDB interface module

-- add local load path
userHome = os.getenv('HOME')
package.cpath = package.cpath .. ";" .. userHome .. "/.config/lualibs/?.so"

-- require xresources C module
require 'xresources'
require 'string'

-- module index
local module_xrdb = {}

-- query the xrdb database for a resource of type 'string'
local function xrdb_get_string(resource_name)
  return xrdb_get_resource(resource_name, 'String')
end

-- query the xrdb database for a resource of type 'long'
local function xrdb_get_int(resource_name)
  return xrdb_get_resource(resource_name, 'Integer')
end

-- query the xrdb database for a resource of type 'bool'
local function xrdb_get_float(resource_name)
  return xrdb_get_resource(resource_name, 'Float')
end

local function parse_string_resource(resource_name)
  -- search, match and replace the # character at the start of an xresource value
  r_str = string.gsub(xrdb_get_string(resource_name), "^#", "0x", 1)
  return r_str
end

local function xrdb_set(resource_name, resource_value)
  return xrdb_set_resource(resource_name, resource_value)
end

-- map lua functions
module_xrdb.parse_string_resource = parse_string_resource
module_xrdb.get_string = xrdb_get_string
module_xrdb.get_integer = xrdb_get_int
module_xrdb.get_float = xrdb_get_float
module_xrdb.set = xrdb_set

-- return module
return module_xrdb

--
