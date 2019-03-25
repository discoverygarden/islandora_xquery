<?php

interface IslandoraXQueryQueryInterface {


  /**
   * Simple factory method.
   *
   * @param string $context
   *   A file name to provide as the context item to the query.
   * @param string $output
   *   A file name to which to write the output of the query.
   * @param string $query
   *   A file containing the query to run.
   *
   * @return IslandoraXQueryQueryInterface
   *   An instance of an object implementing this interface.
   */
  public static function createBasicQuery($context, $output, $query);

  /**
   * Helper; determine if all required executables are present.
   *
   * @return bool
   *   TRUE if they are present; otherwise, FALSE.
   */
  public static function hasExecutable();

  /**
   * Execute the query.
   *
   * @return bool
   *   TRUE on success; FALSE on failure.
   */
  public function execute();

  /**
   * Test that the given query appears to be executable.
   *
   * @return bool
   *   TRUE on success; FALSE on failure.
   */
  public function validate();

}
