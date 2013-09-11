<?php

/**
 * @file
 * Definition of a Bookmark object.
 */

/**
 * Interface defining the base operations required to implement a backend.
 */
interface BookmarkInterface {
  /**
   * Adds the specified PID to the given list.
   *
   * @param string $pid
   *   The PID of an object.
   */
  public function addPid($pid);

  /**
   * Get PIDs associated with this list.
   *
   * @return array
   *   An array of string, each representing an ID in a list.
   */
  public function getPids($offset = 0, $limit = -1);

  /**
   * Removes the specified PID from the given list.
   *
   * @param string $pid
   *   The PID of an object.
   */
  public function removePid($pid);

  /**
   * Shares a list with the given user.
   *
   * @param int $user
   *   A user ID.
   */
  public function addUser($user);

  /**
   * Get a list of all the users associated with a list.
   *
   * @return array
   *   An array of integers, representing user IDs.
   */
  public function getUsers();

  /**
   * Removes the given user from the object.
   *
   * @param int $user
   *   A user ID.
   */
  public function removeUser($user);

  /**
   * Delete the given list.
   *
   * Removes the list from the backend; attempting to get the same list again
   * should fail.
   */
  public function delete();
}

/**
 * Class that defines a Bookmark object. A Bookmark object needs only to be
 * constructed such that it contains a listname and listid for uniqueness.
 */
abstract class Bookmark implements BookmarkInterface {
  /**
   * The name of the object.
   *
   * @var string
   */
  public $bookmarkName;

  /**
   * The owner of the object.
   *
   * @var integer
   */
  public $bookmarkOwner;

  /**
   * The ID of the given list.
   *
   * @var mixed
   */
  protected $bookmarkId;

  /**
   * Constructor for the Bookmark object.
   */
  public function __construct($id, $name, $owner) {
    $this->bookmarkId = $id;
    $this->bookmarkName = $name;
    $this->bookmarkOwner = $owner;
  }

  /**
   * Accessor for the list ID.
   *
   * @return mixed
   *   The value of $this->bookmarkId.
   */
  public function getId() {
    return $this->bookmarkId;
  }

  /**
   * Get the given (existing) list.
   *
   * @param mixed $id
   *   The ID of the list to attempt to load.
   * @param bool $force_session_attempt
   *   Force examination of the session, to handle the hook_user_login() stuff.
   *
   * @return Bookmark|NULL
   *   Returns either an instantiated Bookmark object if the list exists, or
   *   NULL if we could not get it.
   */
  public static function getList($id, $force_session_attempt = FALSE) {
    global $user;

    $bookmark = NULL;
    if ($user->uid === 0 || $force_session_attempt) {
      $bookmark = BookmarkSession::getList($id, TRUE);
    }

    if ($bookmark === NULL) {
      // Pass off to BookmarkDatabase.
      $bookmark = BookmarkDatabase::getList($id, FALSE);
    }

    return $bookmark;
  }

  /**
   * Get the number of PIDs in the current list.
   *
   * Individual backends could probably do this more efficiently, so feel free
   * to override.
   *
   * @return int
   *   An integer representing the number of items in the current list.
   */
  public function getPidCount() {
    return count($this->getPids());
  }

  /**
   * Remove multiple PIDs from the list.
   *
   * Individual backends could probably do this more efficiently, so feel free
   * to override.
   *
   * @param array $pids
   *   An array of pids.
   */
  public function removePids($pids) {
    foreach ($pids as $pid) {
      $this->removePid($pid);
    }
  }

  /**
   * Remove all PIDs from the list.
   *
   * Individual backends could probably do this more efficiently, so feel free
   * to override.
   */
  public function removeAllPids() {
    $this->removePids($this->getPids());
  }

  /**
   * Remove multiple users from the list.
   *
   * Individual backends could probably do this more efficiently, so feel free
   * to override.
   *
   * @param array $users
   *   An array of integers, each representing a user ID.
   */
  public function removeUsers($users) {
    foreach ($users as $user) {
      $this->removeUser($user);
    }
  }

  /**
   * Adds the specified pids to the object and either the database or session.
   *
   * Individual backends could probably do this more efficiently, so feel free
   * to override.
   *
   * @param array $pids
   *   An array of strings, representing object PIDs.
   */
  public function addPids($pids) {
    $pids = array_filter($pids, 'islandora_object_load');
    foreach ($pids as $pid) {
      $this->addPid($pid);
    }
  }

  /**
   * Adds the specified users to the object and either the database or session.
   *
   * Individual backends could probably do this more efficiently, so feel free
   * to override.
   *
   * @param array $users
   *   An array of integers, representings user IDs.
   */
  public function addUsers($users) {
    foreach ($users as $user) {
      $this->addUser($user);
    }
  }

  /**
   * Create a new list for the current user.
   *
   * @param string $name
   *   A name for the newly created list.
   */
  public static function createNewList($name) {
    global $user;

    if ($user->uid === 0) {
      return BookmarkSession::createNewList($name);
    }
    else {
      return BookmarkDatabase::createNewList($name);
    }
  }

  /**
   * Removes the list from the backend.
   *
   * @deprecated Just here to conform with the old API.
   */
  public function removeList() {
    $this->delete();
  }

  /**
   * Changes the name of the object in the session or the database.
   *
   * @param string $name
   *   The new name of the object.
   */
  public function changeListName($name) {
    $this->bookmarkName = $name;
  }

  /**
   * Check if user can execute management functions on the current Bookmark.
   *
   * @return bool
   *   TRUE if they can execute management functions, FALSE otherwise.
   */
  public function managementAccess() {
    global $user;
    if ($user->uid === $this->bookmarkOwner) {
      return TRUE;
    }
    else {
      return FALSE;
    }
  }
}
