<?php

class IslandoraXqueryNoExecutableException extends Exception {
  protected $name;
  protected $class;

  public function setImplementationInfo($name, $class = NULL) {
    $this->name = $name;
    $this->class = $class;
    return $this;
  }

  public function getImplementationInfo() {
    return array(
      $this->name,
      $this->class,
    );
  }
}
