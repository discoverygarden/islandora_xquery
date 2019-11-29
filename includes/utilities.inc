<?php

/**
 * @file
 * This file contains helper functions used throughout this module.
 */

/**
 * Gets the versions of the given zorba executable.
 *
 * @param string $executable_path
 *   The absolute path to zorba executable, if not provided the variable
 *   "islandora_xquery_zorba" will be used instead.
 *
 * @return string|bool
 *   The version of the zorba executable if successful, FALSE otherwise.
 */
function islandora_xquery_get_zorba_version($executable_path = NULL) {
  $executable_path = $executable_path ? $executable_path : variable_get('islandora_xquery_zorba', ISLANDORA_XQUERY_ZORBA_DEFAULT_LOCATION);
  if (is_executable($executable_path)) {
    $ret = -1;
    $output = array();
    $command = "$executable_path --version 2>&1";
    exec($command, $output, $ret);
    // Odd behavior in that the Zorba executable returns 1 on success when
    // checking for the version number.
    if ($ret == 1 && isset($output[0])) {
      // Example return value "Zorba NoSQL Query Processor, Version: 3.0.0".
      $matches = array();
      if (preg_match('/\d\.\d\.\d/', $output[0], $matches)) {
        return array_pop($matches);
      }
    }
  }
  return FALSE;
}

/**
 * Gets the versions of the given basex executable.
 *
 * @param string $executable_path
 *   The absolute path to the basex executable. If not provided the
 *   "islandora_xquery_basex" variable will be used instead.
 *
 * @return bool
 *   Whether the basex executable exists at the given path.
 */
function islandora_xquery_get_basex_version($executable_path = NULL) {
  $executable_path = $executable_path ? $executable_path : variable_get('islandora_xquery_basex', ISLANDORA_XQUERY_BASEX_DEFAULT_LOCATION);
  if (is_executable($executable_path)) {
    $ret = -1;
    $output = array();
    $command = "$executable_path -h 2>&1";
    exec($command, $output, $ret);
    if ($ret == 1 && $output) {
      // There might be some warnings before the line with version info, so
      // loop through the output.
      foreach ($output as $line) {
        // Example output: "BaseX 8.5.1 [Standalone]".
        if (strpos($line, 'BaseX') === 0) {
          $matches = array();
          if (preg_match('/\d\.\d\.\d/', $line, $matches)) {
            return array_pop($matches);
          }
        }
      }
    }
  }
  return FALSE;
}
