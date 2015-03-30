// The JobCache is an object that allows the storage and retrieval of
// JavaScript objects. If sessionStorage is supported by the browser, the cache
// will persist across page loads, but not across different windows and tabs.
//
// It is simple to add, retrieve, and remove objects from the JobCache.
//
//     > var myObject = {'foo': 'bar'};
//     > var key = 'MyKey';
//     > JobCache.setItem(key, myObject);
//     > JobCache.getItem(key);
//     {'foo': 'bar'}
//     > JobCache.removeItem(key)
//
// The JobCache is a wrapper around the sessionStorage API, automatically JSON
// encoding and decoding objects so that they can be stored as strings, the
// only format storable per the sessionStorage spec.
//
// If sessionStorage is not supported by the browser, a basic JavaScript object
// is used as an alternative 'backend', but the API remains identical.
//
// https://developer.mozilla.org/en-US/docs/Web/API/Window/sessionStorage
var JobCache = (function(window, undefined) {
  'use strict';

  var hasSessionStorage = !!window.sessionStorage;
  // Fall back to object-based storage if sessionStorage is not available
  var store = window.sessionStorage || {};
  // Need to keep track of insertion order if sessionStorage is not available
  var keys = [];

  // See sessionStorage.key
  var key = function(index) {
    if (hasSessionStorage === true) {
      return store.key(index);
    } else {
      var value = keys[index];
      if (value === undefined) {
        // Match the sessionStorage return value
        return null;
      }
      return value;
    }
  };

  // Retrieve the object value referenced by key
  // Accepts:
  //   Key string
  // Returns:
  //   The object value referenced by key, if key is known to the store, else
  //   null
  var getItem = function(key) {
    var value;
    if (hasSessionStorage === true) {
      value = store.getItem(key);
    } else {
      value = store[key];
    }
    if (value === null || value === undefined) {
      return null;
    }
    return JSON.parse(value);
  };

  // Store the value object and reference it with key
  //
  // The value is assumed to be JSON encodable, and is stored
  // as a JSON string
  // Accepts:
  //   key: Key to store the value under
  //   value: Object to store
  // Returns:
  //   undefined
  var setItem = function(key, value)  {
    if (hasSessionStorage === true) {
      store.setItem(key, JSON.stringify(value));
    } else {
      store[key] = JSON.stringify(value);
      keys.push(key);
    }
  };

  // See sessionStorage.removeItem
  var removeItem = function(key) {
    if (hasSessionStorage === true) {
      store.removeItem(key);
    } else {
      var index = keys.indexOf(key);
      keys.splice(index, 1);
      delete store[key];
    }
  };

  // See sessionStorage.clear
  var clear = function() {
    if (hasSessionStorage === true) {
      store.clear();
    } else {
      store = {};
      keys = [];
    }
  };

  var exported = {
    key: key,
    getItem: getItem,
    setItem: setItem,
    removeItem: removeItem,
    clear: clear
  };

  // Define the length property on the object backend
  if (hasSessionStorage !== true) {
    Object.defineProperty(exported, 'length', {
      get: function() {
        return keys.length;
      }
    });
  }

  return exported;
})(window);
