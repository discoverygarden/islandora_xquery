<?php

/**
 * @file
 * DB utility functionality for Islandora XQuery.
 */

/**
 * Get the ID for the given configuration.
 *
 * @param string $machine_name
 *   The machine name of the configuration ID we are grabbing.
 *
 * @return bool|int
 *   The configuration ID if it exists; otherwise, FALSE.
 */
function islandora_xquery_retrieve_association_from_machine_name($machine_name) {
  $result = db_query('SELECT association_id from {islandora_xquery_associations} WHERE machine_name = :machine_name', array(':machine_name' => $machine_name));
  $config_id = $result->fetchColumn();
  return $config_id;
}

/**
 * Get the name for the given association.
 *
 * @param int $association_id
 *   The ID for a configuration for which to grab the name.
 *
 * @return string
 *   The name of the configuration.
 */
function islandora_xquery_retrieve_association_name($association_id) {
  $result = db_query('SELECT template_name from {islandora_xquery_associations} WHERE association_id = :association_id', array(':association_id' => $association_id));
  $config_name = $result->fetchColumn();
  return $config_name;
}

/**
 * Create a new configuration.
 *
 * @param string $template_name
 *   The name of a configuration to add the the DB.
 * @param string $machine_name
 *   The unique machine name to add the the DB.
 * @param string $template_description
 *   An optional description about the template.
 *
 * @return int
 *   The ID with which the configuration was created in the database.
 */
function islandora_xquery_add_association($template_name, $machine_name, $template_description) {
  $id = db_insert('islandora_xquery_associations')
    ->fields(
      array(
        'template_name' => $template_name,
        'machine_name' => $machine_name,
        'template_description' => empty($template_description) ? NULL : $template_description,
      )
    )
    ->execute();
  return $id;
}

/**
 * Check that a association exists with the given ID.
 *
 * @param int $association_id
 *   An ID to test.
 *
 * @return bool
 *   Whether or not the ID exists.
 */
function islandora_xquery_association_exists($association_id) {
  $result = db_query('SELECT COUNT(*) FROM {islandora_xquery_associations} WHERE association_id = :association_id', array(':association_id' => $association_id));
  return intval($result->fetchField()) > 0;
}

/**
 * Get all configurations available.
 *
 * @return array
 *   An array of associative arrays, each of which contains:
 *   - name: The name of the given configuration.
 *   - id: The ID of the given configuration.
 */
function islandora_xquery_get_associations() {
  $associations = array();
  $result = db_query('SELECT * FROM {islandora_xquery_associations}');
  foreach ($result as $record) {
    $associations[] = array(
      'template_name' => $record->template_name,
      'association_id' => $record->association_id,
      'machine_name' => $record->machine_name,
      'template_description' => $record->template_description,
    );
  }
  return $associations;
}

/**
 * Get all fields in the given template.
 *
 * @param int $machine_name
 *   The machine name for the template for which to get the list of tokens.
 *
 * @return array
 *   An associative array of associative arrays, representing tokens
 *   mapped to the details about the given tokens, including:
 *   - association_id
 *   - token_name
 *   - token_description
 *   - token_default
 */
function islandora_xquery_get_tokens_from_machine_name($machine_name) {
  $query = db_select('islandora_xquery_tokens', 't')
    ->fields('t', array());
  $association_alias = $query->join('islandora_xquery_associations', 'a', 'a.association_id = t.association_id');

  return $query->condition("$association_alias.machine_name", $machine_name)
    ->execute()
    ->fetchAllAssoc('token_name', PDO::FETCH_ASSOC);
}

/**
 * Get all fields in the given template.
 *
 * @param int $association_id
 *   The ID for the template for which to get the list of tokens.
 *
 * @return array
 *   An associative array of associative arrays, representing tokens
 *   mapped to the details about the given tokens, including:
 *   - association_id
 *   - token_name
 *   - token_description
 *   - token_default
 */
function islandora_xquery_get_tokens($association_id) {
  $result = db_query('SELECT * FROM {islandora_xquery_tokens} WHERE association_id = :association_id', array(':association_id' => $association_id));
  $tokens = $result->fetchAllAssoc('token_name', PDO::FETCH_ASSOC);
  return $tokens;
}

/**
 * Gets the XQuery value for a template.
 *
 * @param string $machine_name
 *   The machine name for the association to get the XQuery for.
 */
function islandora_xquery_get_xquery_from_machine_name($machine_name) {
  $query = db_select('islandora_xquery_xquery', 'x')
    ->fields('x', array('xquery'));
  $association_alias = $query->join('islandora_xquery_associations', 'a', 'a.association_id = x.association_id');

  return $query->condition("$association_alias.machine_name", $machine_name)
    ->execute()
    ->fetchField();
}

/**
 * Gets the XQuery value for a template.
 *
 * @param int $association_id
 *   The ID for the association to get the XQuery for.
 */
function islandora_xquery_get_xquery($association_id) {
  $result = db_query('SELECT xquery from {islandora_xquery_xquery} WHERE association_id = :association_id', array(':association_id' => $association_id));
  $xquery = $result->fetchColumn();
  return $xquery;
}

/**
 * Add fields to a specified association ID.
 *
 * @param int $association_id
 *   The configuration ID of the configuration which to add to.
 * @param array $tokens
 *   An associative array of associative arrays, containing details about the
 *   given token being added, including:
 *   - token_name
 *   - token_description
 *   - token_default
 */
function islandora_xquery_add_tokens($association_id, $tokens) {
  foreach ($tokens as $token) {
    $token['association_id'] = $association_id;
    db_insert('islandora_xquery_tokens')
      ->fields(array(
          'association_id',
          'token_name',
          'token_description',
          'token_default',
        )
      )
      ->values($token)
      ->execute();
  }
}

/**
 * Adds the XQuery for a template to the database.
 *
 * @param int $association_id
 *   The ID of the association we are modifying.
 * @param string $xquery
 *   A string of XQuery to be used in the template.
 */
function islandora_xquery_add_xquery($association_id, $xquery) {
  db_insert('islandora_xquery_xquery')
    ->fields(array(
      'association_id',
      'xquery',
      )
    )
    ->values(array(
        'association_id' => $association_id,
        'xquery' => $xquery,
      )
    )
    ->execute();
}

/**
 * Updates the XQuery for a template in the database.
 *
 * @param int $association_id
 *   The ID of the association we are modifying.
 * @param string $xquery
 *   A string of XQuery to be used in the template.
 */
function islandora_xquery_update_xquery($association_id, $xquery) {
  db_update('islandora_xquery_xquery')
    ->fields(array(
      'xquery' => $xquery,
    ))
    ->condition('association_id', $association_id)
    ->execute();
}

/**
 * Deletes the given tokens out of the database.
 *
 * @param int $association_id
 *   The ID of the association we are modifying.
 * @param array $tokens
 *   An associative array of associative arrays, containing details about the
 *   given token being added, including:
 *   - token_name
 *   - token_description
 *   - token_default
 */
function islandora_xquery_delete_tokens($association_id, $tokens) {
  db_delete('islandora_xquery_tokens')
    ->condition('token_name', array_keys($tokens))
    ->condition('association_id', $association_id)
    ->execute();
}

/**
 * Deletes all references to an association in the database.
 *
 * @param int $association_id
 *   The ID of the association we are modifying.
 */
function islandora_xquery_delete_association($association_id) {
  db_delete('islandora_xquery_tokens')
    ->condition('association_id', $association_id)
    ->execute();
  db_delete('islandora_xquery_xquery')
    ->condition('association_id', $association_id)
    ->execute();
  db_delete('islandora_xquery_associations')
    ->condition('association_id', $association_id)
    ->execute();
}
