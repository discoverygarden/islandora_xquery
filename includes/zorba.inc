<?php

/**
 * @file
 * Defines a class for interacting with Zorba.
 */

namespace Islandora\Zorba;
use ArrayAccess,
    InvalidArgumentException,
    OutOfBoundsException;

/**
 * Class Options
 *
 * Models options for the Zorba CLI.
 *
 * @package Islandora\Zorba
 */
class Options implements ArrayAccess {

  /**
   * Defines the possible options and what values are acceptable.
   *
   * Each array key represents a valid option for the Zorba CLI, its value is
   * must be an associative array with the following keys:
   *  - type: Expected to be either 'boolean', 'string', 'integer', 'select',
   *    the type defines default validation and serialization functions for the
   *    option. Required.
   *  - multiple: If present the option will be stored as an array and the user
   *    will be add multiple values for the option. Optional.
   *  - options: An array of values that the option is limited to, this field is
   *    required when the type is 'select'. For other types it has no affect.
   *  - validate: A function that takes an option name and value and returns a
   *    boolean value indicating if that option value pair is valid.
   *  - serialize: A function that takes an option name and value and returns
   *    the string representation of that option to pass to the Zorba CLI.
   *
   * Some assumptions are made, about how each option will be serialized and
   * validated based on it's type. Options can give explicit validation /
   * serialization functions though.
   *
   * @var array
   */
  protected static $optionDefinitions = array(
    // Print timing information. In case of multiple queries the timing
    // information is provided per each query. Both wall clock time and user
    // time (which excludes I/O, network delays and other kernel waits) are.
    'timing' => array(
      'type' => 'boolean',
    ),
    // Write the result to the given file.
    'output-file' => array(
      'type' => 'string',
    ),
    // Set serialization parameter in the form of a key value pairs see the
    // @link documentation here @endlink
    // @link http://www.w3.org/TR/xslt-xquery-serialization/#serparam
    'serialization-parameter' => array(
      'type' => 'string',
      'multiple' => TRUE,
    ),
    // Serialize the result as HTML.
    'serialize-html' => array(
      'type' => 'boolean',
    ),
    // Serialize the result as Text.
    'serialize-text' => array(
      'type' => 'boolean',
    ),
    // Indent output.
    'indent' => array(
      'type' => 'boolean',
    ),
    // Print the queries.
    'print-query' => array(
      'type' => 'boolean',
    ),
    // Print the errors as XML.
    'print-errors-as-xml' => array(
      'type' => 'boolean',
    ),
    // Set the byte-order-mark for the serializer.
    'byte-order-mark' => array(
      'type' => 'string',
    ),
    // Omit the XML declaration from the result.
    'omit-xml-declaration' => array(
      'type' => 'boolean',
    ),
    // Set the base URI property of the static context.
    'base-uri' => array(
      'type' => 'string',
    ),
    // Set the boundary-space policy ('strip' or 'preserve') in
    // the static context.
    'boundary-space' => array(
      'type' => 'select',
      'options' => array('strip', 'preserve'),
    ),
    // Add the given collation and set the value of the
    // default collation in the static context to the given collation.
    // @see http://www.w3.org/TR/xquery/#id-default-collation-declaration
    'default-collation' => array(
      'type' => 'string',
    ),
    // Set the construction mode ('strip' or 'preserve') in
    // the static context.
    'construction-mode' => array(
      'type' => 'select',
      'options' => array('strip', 'preserve'),
    ),
    // Set the ordering mode ('ordered' or 'unordered') in the
    // static context.
    'ordering-mode' => array(
      'type' => 'select',
      'options' => array('ordered', 'unordered'),
    ),
    // Execute the given queries multiple times.
    'multiple' => array(
      'type' => 'integer',
    ),
    // Query specified as file path to a file containing a valid Zorba XQuery.
    'query' => array(
      'type' => 'string',
      'multiple' => TRUE,
    ),
    // Provide the value for a variable given a file
    // (name=file) or a value (name:=value)
    'external-variable' => array(
      'type' => 'string',
      'multiple' => TRUE,
    ),
    // Set the context item to the XML document in a given file.
    'context-item' => array(
      'type' => 'string',
    ),
    // Optimization level for the query compiler
    // (O0, O1 or O2 - default: O1)
    'optimization-level' => array(
      'type' => 'select',
      'options' => array('O0', 'O1', 'O2'),
    ),
    // Query compiler option to treat the query as a library module.
    // If this is set --compile-only option is also set to true.
    'lib-module' => array(
      'type' => 'boolean',
    ),
    // Stop after parsing the query.
    'parse-only' => array(
      'type' => 'boolean',
    ),
    // Only compile (don't execute).
    'compile-only' => array(
      'type' => 'boolean',
    ),
    // Do not serialize (discard) result.
    'no-serializer' => array(
      'type' => 'boolean',
    ),
    // Launch the Zorba debugger server and connect to a DBGP-enabled
    // debugger client.
    'debug' => array(
      'type' => 'boolean',
    ),
    // The host where the DBGP-enabled debugger client listens for
    // connections. Defaults to: 127.0.0.1
    'debug-host' => array(
      'type' => 'string',
    ),
    // The port on which the DBGP-enabled debugger client listens
    // for connections. Defaults to: 28028
    'debug-port' => array(
      'type' => 'integer',
    ),
    // Print no logo when starting.
    'no-logo' => array(
      'type' => 'boolean',
    ),
    // Specify a timeout in seconds. After the specified time, the
    // execution of the query will be aborted.
    'timeout' => array(
      'type' => 'integer',
    ),
    // URI path (list of directories) added to the built-in URI
    // resolver, i.e. where to find modules/schemas to import.
    'uri-path' => array(
      'type' => 'string',
      'multiple' => TRUE,
    ),
    // Library path (list of directories) where Zorba will look for
    // dynamic libraries (e.g., module external function implementations.
    'lib-path' => array(
      'type' => 'string',
      'multiple' => TRUE,
    ),
    // Path (list of directories) to add to both the URI and
    // Library paths.
    'module-path' => array(
      'type' => 'string',
      'multiple' => TRUE,
    ),
    // JVM classpath to be used by modules using Java implementations.
    'classpath' => array(
      'type' => 'string',
      'multiple' => TRUE,
    ),
    // Set an XQuery option in the static context. The QName of the
    // option is passed as a string in the notation by James Clark
    // (i.e. {namespace}localname).
    // For example, --option {http://zorba.io/}option=value
    'option' => array(
      'type' => 'string',
      'multiple' => TRUE,
    ),
    // Output a trailing newline after the result of the query.
    'trailing-nl' => array(
      'type' => 'boolean',
    ),
    // Mapping specifying a stop-words URI to another.
    'stop-words' => array(
      'type' => 'string',
      'multiple' => TRUE,
    ),
    // Mapping specifying a thesaurus URI to another.
    'thesaurus' => array(
      'type' => 'string',
      'multiple' => TRUE,
    ),
    // Serialize and then load the query execution plan.
    'serialize-plan' => array(
      'type' => 'boolean',
    ),
    // Do not execute the query; just compile it and save the
    // execution plan in the file specified with the -o option.
    'compile-plan' => array(
      'type' => 'boolean',
    ),
    // Do not compile the query; instead load the execution plan
    // from the file specified by the -f -q options (or by any file specified
    // without any other argument), and execute the loaded plan.
    'execute-plan' => array(
      'type' => 'boolean',
    ),
    // Do not use HTTP to resolve URIs.
    'disable-http-resolution' => array(
      'type' => 'boolean',
    ),
  );

  /**
   * Options for the Zorba CLI.
   *
   * The options in this array must conform to the restrictions given in
   * $valid_options.
   *
   * @var array
   */
  protected $options = array();

  /**
   * Defines default option definitions based on the options type.
   *
   * The option definitions can optionally overwrite these defaults.
   *
   * @return array
   *   The default option definitions for all accepted types.
   */
  protected static function getDefaultTypeOptionDefinitions() {
    // @codingStandardsIgnoreStart
    static $default_option_type_definitions = NULL;
    if ($default_option_type_definitions === NULL) {
      // Call the given function with only the $value parameter. Used in the
      // simple validation callbacks.
      $call_without_option_function = function ($function) {
        return function ($option, $value) use ($function) {
          return $function($value);
        };
      };
      // Serialize a single value option.
      $serialize_single_value_option = function ($option, $value) {
        return "--{$option} \"{$value}\"";
      };
      $default_option_type_definitions = array(
        'boolean' => array(
          'validate' => $call_without_option_function('is_bool'),
          'serialize' => function ($option, $value) {
            return $value ? "--{$option}" : '';
          },
        ),
        'string' => array(
          'validate' => $call_without_option_function('is_string'),
          'serialize' => $serialize_single_value_option,
        ),
        'integer' => array(
          'validate' => $call_without_option_function('is_int'),
          'serialize' => $serialize_single_value_option,
        ),
        'select' => array(
          'validate' => function ($option, $value) {
            return in_array($value, Options::$optionDefinitions[$option]['options']);
          },
          'serialize' => $serialize_single_value_option,
        ),
      );
    }
    return $default_option_type_definitions;
    // @codingStandardsIgnoreEnd
  }

  /**
   * Gets only the option defaults for the given type.
   *
   * @param string $type
   *   The type of option to get the defaults for.
   *
   * @return array
   *   The default option definition for the given type.
   */
  protected static function getDefaultTypeOptionDefinition($type) {
    $defaults = self::getDefaultTypeOptionDefinitions();
    return $defaults[$type];
  }

  /**
   * Checks to see if the given option exists for Zorba.
   *
   * @param string $option
   *   The option to check.
   *
   * @return bool
   *   TRUE if the given option exists, FALSE otherwise.
   */
  public static function exists($option) {
    return !empty(self::$optionDefinitions[$option]);
  }

  /**
   * Checks to see if the given option / value is valid.
   *
   * @param string $option
   *   The option to check.
   * @param mixed $value
   *   The option value to check.
   *
   * @return bool
   *   TRUE if the given option / value is valid, FALSE otherwise.
   */
  public static function valid($option, $value) {
    if (self::exists($option)) {
      $definition = self::$optionDefinitions[$option];
      $definition += self::getDefaultTypeOptionDefinition($definition['type']);
      return call_user_func($definition['validate'], $option, $value);
    }
    return FALSE;
  }

  /**
   * Checks to see if the given option allows multiple values.
   *
   * @param string $option
   *   The option to check.
   *
   * @return bool
   *   TRUE if the given option allows multiple values, FALSE otherwise.
   */
  public static function multiple($option) {
    return isset(self::$optionDefinitions[$option]['multiple']) && self::$optionDefinitions[$option]['multiple'];
  }

  /**
   * Serialize the given option and it's value to a string to pass to the CLI.
   *
   * @param string $option
   *   The option to serialize.
   * @param mixed $value
   *   The option value to serialize.
   *
   * @return string
   *   A string representing the given option and value.
   */
  public static function serialize($option, $value) {
    if (self::exists($option)) {
      $definition = self::$optionDefinitions[$option];
      $definition += self::getDefaultTypeOptionDefinition($definition['type']);
      // Serialize the given value using the serialize function in the option
      // definition.
      $serialize = function($value) use($option, $definition) {
        return call_user_func($definition['serialize'], $option, $value);
      };
      $multiple = self::multiple($option) && is_array($value);
      return ($multiple) ? implode(' ', array_map($serialize, $value)) : $serialize($value);
    }
    return FALSE;
  }

  /**
   * Create an new Islandora\Zorba\Options object.
   *
   * @throws InvalidArgumentException
   *
   * @param array $options
   *   Options to use. For what options are available see
   *   http://www.zorba.io/documentation/latest/zorba/cli when passing options
   *   here you can omit the '--'. Options which can be specified multiple times
   *   are expected to be arrays.
   */
  public function __construct(array $options = array()) {
    foreach ($options as $option => $value) {
      // offsetSet() will throw an exception if the option or value is not
      // valid.
      $this[$option] = $value;
    }
  }

  /**
   * Magic offsetGet.
   *
   * This is implemented for multivalued options as offsetGet does not return by
   * reference, which is needed for multidimensional arrays.
   *
   * @see ArrayAccess::offsetGet
   */
  public function &__get($option) {
    if (isset($this->options[$option])) {
      return $this->options[$option];
    }
    else {
      throw new OutOfBoundsException($option);
    }
  }

  /**
   * Magic offsetSet.
   *
   * This is implemented for multivalued options as offsetGet does not return by
   * reference, which is need for multidimensional arrays.
   *
   * @see ArrayAccess::offsetGet
   */
  public function __set($option, $value) {
    $this->offsetSet($option, $value);
  }

  /**
   * Checks to see if the offset exists.
   *
   * @see ArrayAccess::offsetExists
   */
  public function offsetExists($option) {
    return isset($this->options[$option]);
  }

  /**
   * Custom offsetGet.
   *
   * We don't allow for indirect access to elements as our min version of PHP is
   * 5.3.3, and indirect access was not added until 5.3.4. Please use the __get
   * to do indirect access on options.
   * @see http://php.net/manual/en/arrayaccess.offsetget.php
   *
   * @see ArrayAccess::offsetGet
   */
  public function offsetGet($option) {
    if (isset($this->options[$option])) {
      return $this->options[$option];
    }
    else {
      throw new OutOfBoundsException($option);
    }
  }

  /**
   * Custom offsetSet.
   *
   * @see ArrayAccess::offsetSet
   */
  public function offsetSet($option, $value) {
    // Check if the $option is valid.
    if (!self::exists($option)) {
      throw new InvalidArgumentException('Unknown command line option ' . $option);
    }
    // Validate the given single value.
    $validate = function($value) use($option) {
      if (!Options::valid($option, $value)) {
        throw new InvalidArgumentException("Invalid value '{$value}' given for command line option '{$option}'");
      }
    };
    // Multiple valued options must always be stored as arrays.
    $value = (self::multiple($option) && !is_array($value)) ? array($value) : $value;
    // Validate the given value / values.
    is_array($value) ? array_walk($value, $validate) : $validate($value);
    // No exceptions? Assign.
    $this->options[$option] = $value;
  }

  /**
   * Custom offsetUnset.
   *
   * @see ArrayAccess::offsetUnset
   */
  public function offsetUnset($option) {
    unset($this->options[$option]);
  }

  /**
   * Enables the given option.
   *
   * @throws InvalidArgumentException
   *   If the given option can not be enabled.
   *
   * @param string $option
   *   Only use options which can be toggled on/off like "print-query".
   */
  public function enable($option) {
    if (!self::valid($option, TRUE)) {
      throw new InvalidArgumentException("{$option} can not be enabled");
    }
    $this->options[$option] = TRUE;
  }

  /**
   * Disables the given option.
   *
   * @throws InvalidArgumentException
   *   If the given option can not be disabled.
   *
   * @param string $option
   *   Only use options which can be toggled on/off like "print-query".
   */
  public function disable($option) {
    if (!self::valid($option, FALSE)) {
      throw new InvalidArgumentException("{$option} can not be disabled");
    }
    unset($this->options[$option]);
  }

  /**
   * Sets the given option to the given value.
   *
   * @throws InvalidArgumentException
   *   If the given option can not be enabled.
   *
   * @param string $option
   *   The option to set.
   *
   * @param mixed $value
   *   The value to set the option.
   */
  public function set($option, $value) {
    if (!self::valid($option, $value)) {
      throw new InvalidArgumentException("{$option} can not be set to {$value}");
    }
    $this->options[$option] = $value;
  }

  /**
   * Generates a string representation.
   *
   * @return string
   *   The representation of this object.
   */
  public function __toString() {
    $output = '';
    foreach ($this->options as $option => $value) {
      $output .= self::serialize($option, $value) . ' ';
    }
    return trim($output);
  }

}

/**
 * Calls the Zorba CLI with the stored options.
 *
 * @param Options $options
 *   The options to pass to the Zorba CLI.
 *
 * @return array
 *   The output of the command executed, as returned by exec().
 */
function execute(Options $options) {
  $executable_path = variable_get('islandora_xquery_zorba', '/usr/bin/zorba');
  if (is_executable($executable_path)) {
    $ret = -1;
    $output = array();
    // From the Zorba 3.0 documentation:
    // --as-files treats all -q arguments as file paths instead of URIs or
    // inline queries. This option is deprecated and will be defaulted to true
    // in the future, so any entry in the command line is going to be treated as
    // files.
    // Therefore for future compatibility we assume that --as-files will always
    // be enabled.
    $command = "$executable_path --as-files {$options}";
    exec($command, $output, $ret);
    if ($ret == 0) {
      return $output;
    }
  }
  return FALSE;
}

/**
 * Checks if the given options result in a valid query.
 *
 * @param Options $options
 *   Options to validate against.
 *
 * @return bool
 *   TRUE if the options result in a parse-able query.
 */
function valid(Options $options) {
  $options = clone $options;
  $options->enable('parse-only');
  $output = execute($options);
  return $output !== FALSE;
}
