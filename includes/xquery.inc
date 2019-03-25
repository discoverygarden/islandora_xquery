<?php

class IslandoraXquery {

  public static function queryObject($context, $output, $query) {
    $class = static::getImplementation()[1];
    return call_user_func_array(array($class, 'createBasicQuery'), func_get_args());
  }

  /**
   * List the implementations.
   *
   * @return array
   *   A mapping of machine names to class names implementing
   *   IslandoraXQueryQueryInterface.
   */
  public static function getImplementations() {
    module_load_include('inc', 'islandora_xquery', 'includes/zorba');
    module_load_include('inc', 'islandora_xquery', 'includes/basex');
    return array(
      'zorba' => \Islandora\Zorba\Options::class,
      'basex' => \Islandora\Xquery\BaseX::class,
    );
  }

  /**
   * Get info for the selected implementation.
   */
  public static function getImplementation() {
    $implementations = static::getImplementations();
    $selected = variable_get('islandora_xquery_implementation', 'basex');
    if (!isset($implementations[$selected])) {
      throw (new IslandoraXqueryNoExecutableException(t('The selected XQuery processor (@processor) does not appear to be defined.', array(
        '@processor' => $selected,
      ))))
        ->setImplementationInfo($selected);
    }
    elseif (!call_user_func("{$implementations[$selected]}::hasExecutable")) {
      throw (new IslandoraXqueryNoExecutableException(t('The selected XQuery processor (@processor) does not appear to be executable.', array(
        '@processor' => $selected,
      ))))
        ->setImplementationInfo($selected, $implementations[$selected]);
    }
    return array($selected, $implementations[$selected]);
  }

}
