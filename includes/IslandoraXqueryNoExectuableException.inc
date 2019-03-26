<?php

/**
 * Exception if the executable does not appear to be available.
 */
class IslandoraXqueryNoExecutableException extends Exception {
  protected $name;
  protected $class;

  /**
   * Setter for contextual info.
   */
  public function setImplementationInfo($name, $class = NULL) {
    $this->name = $name;
    $this->class = $class;
    return $this;
  }

  /**
   * Getter for contextual info.
   */
  public function getImplementationInfo() {
    return array(
      $this->name,
      $this->class,
    );
  }

}
