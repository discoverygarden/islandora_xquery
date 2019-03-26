<?php

/**
 * Extended interface with form helpers.
 */
interface IslandoraXqueryConfigurableProcessorInterface extends IslandoraXqueryQueryInterface {

  /**
   * Return a small configuraton form.
   *
   * The form should be tree-like, using $offset to access items in $form_state
   * if necessary.
   *
   * @param array $offset
   *   The offset to the returned form segment in the form to which we will be
   *   added.
   * @param array $form_state
   *   State under which we're to be added; probably a good idea to
   *   form_load_include() yourself, if namespaced.
   *
   * @return array
   *   Some elements with which to perform configuration.
   */
  public static function getForm(array $offset, array &$form_state);

  /**
   * Persist the values from our config form.
   *
   * @param array $values
   *   The values which were entered for our part of the form.
   */
  public static function submitForm(array $values);

}
