<?php

/**
 * @file
 * Definition of a Bookmark object.
 */

/**
 * Class that defines a Bookmark object. A Bookmark object needs only to be
 * constructed such that it contains a listname and listid for uniqueness.
 */
class BookmarkSession extends Bookmark {
  /**
   * Place to store the list of keys in the session.
   *
   * @var
   */
  protected $pidList;

  /**
   * Generate the key to use in the session.
   *
   * @param string $name
   *   The name of the list, which we'll likely hash.
   *
   * @return string
   *   A key which should be unique, prefixed with "session_".
   */
  protected static function getKey($name) {
    return format_string('session_!unique', array(
      '!unique' => hash('sha256', $name),
    ));
  }

  /**
   * Constructor for the Bookmark object.
   */
  public function __construct($name) {
    global $user;
    parent::__construct(static::getKey($name), $name, $user->uid);
    $this->pidList = array();
  }

  /**
   * {@inheritdoc}
   */
  public static function getList($id) {
    if(isset($_SESSION['islandora_bookmark'][$id])) {
      return $_SESSION['islandora_bookmark'][$id];
    }
    return NULL;
  }

  /**
   * {@inheritdoc}
   */
  public function getPids($offset = 0, $limit = -1) {
    if ($limit > 0) {
      return array_slice($this->pidList, $offset, $limit);
    }
    else {
      return $this->pidList;
    }
  }

  /**
   * Build a new list.
   *
   * @param string $name
   *   The name for the list.
   * @param object $owner
   *   A user object from which we can grab the uid. If not provided, we'll
   *   grab the global "$user" variable.
   *
   * @return BookmarkDatabase
   *   A new Bookmark object.
   */
  public static function createNewList($name, $owner = NULL) {
    if ($owner === NULL) {
      global $user;
      $owner = $user;
    }

    $list_id = static::getKey($name);

    unset($_SESSION['islandora_bookmark']);
    $_SESSION['islandora_bookmark'][$list_id] = new static($name);

    return static::getList($list_id);
  }

  /**
   * {@inheritdoc}
   */
  public function delete() {
    unset($_SESSION['islandora_bookmark'][$this->bookmarkId]);
  }

  /**
   * {@inheritdoc}
   */
  public function getUsers() {
    return array(
      $this->bookmarkOwner,
    );
  }

  /**
   * {@inheritdoc}
   */
  public function removeUser($user_delete) {
    // No-op.
  }

  /**
   * {@inheritdoc}
   */
  public function addUser($user_add) {
    // No-op.
  }

  /**
   * {@inheritdoc}
   */
  public function removePid($pid) {
    $this->pidList = array_diff($this->pidList, array($pid));
  }

  /**
   * {@inheritdoc}
   */
  public function addPid($pid) {
    $this->pidList[] = $pid;
  }
}
