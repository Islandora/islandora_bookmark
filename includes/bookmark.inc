<?php

/**
 * @file
 * Definition of a Bookmark object.
 */

/**
 * Class that defines a Bookmark object. A Bookmark object needs only to be
 * constructed such that it contains a listname and listid for uniqueness.
 */
class Bookmark {
  /**
   * The list of users who belong to this object.
   *
   * @var array
   */
  public $userList = array();

  /**
   * The list of pids that belong to this object.
   *
   * @var array
   */
  public $pidList = array();

  /**
   * The name of the object.
   *
   * @var string
   */
  public $bookmarkName;

  /**
   * The unique ID of the object. This is generated through the database or
   * explicity assigned when adding objects to the session.
   *
   * @var integer
   */
  public $bookmarkId;

  /**
   * The owner of the object.
   *
   * @var integer
   */
  public $bookmarkOwner;

  /**
   * The user id of the current user accessing the object.
   *
   * @var integer
   */
  public $userId;

  /**
   * Whether the object is created or retrieved through the session.
   *
   * @var boolean
   */
  public $bookmarkSession = FALSE;

  /**
   * Constructor for the Bookmark object.
   */
  public function __construct($params = array()) {
    $this->userId = $params['user_id'];
    $this->userList = $params['bookmark_users'];

    if (!empty($params['bookmark_session'])) {
      $this->bookmarkSession = $params['bookmark_session'];
    }

    if (!empty($params['bookmark_pids'])) {
      $this->pidList = $params['bookmark_pids'];
    }

    if (!empty($params['bookmark_name'])) {
      $this->bookmarkName = $params['bookmark_name'];
    }

    if (!empty($params['bookmark_id'])) {
      $this->bookmarkId = $params['bookmark_id'];
    }

    if (!$params['retrieve']) {
      $this->createList();
    }
    else {
      $this->bookmarkOwner = $params['bookmark_owner'];
    }
  }

  /**
   * Removes the given pids from the object and from the database or session.
   *
   * @param array $pids
   *   An array of pids.
   */
  public function removePids($pids) {
    $this->pidList = array_diff($this->pidList, $pids);

    if (!$this->bookmarkSession) {
      if ($this->managementAccess()) {
        foreach ($pids as $value) {
          $this->removePidDB($value);
        }
      }
      else {
        throw new Exception(t("You do not have access to remove bookmarks from the list @bookmarkname", array("@bookmarkname" => $this->bookmarkName)));
      }
    }
    else {
      $superserial = unserialize($_SESSION['islandora_bookmark'][$this->bookmarkId]);

      foreach ($superserial->pidList as $key => $value) {
        if (in_array($value, $pids)) {
          unset($superserial->pidList[$key]);
          $_SESSION['islandora_bookmark'][$this->bookmarkId] = serialize($superserial);
        }
      }
    }
  }

  /**
   * Remove all pids from the object and either the database or session.
   */
  public function removeAllPids() {
    if (!$this->bookmarkSession) {
      if ($this->managementAccess()) {
        foreach ($this->pidList as $value) {
          $this->removePidDB($value);
        }
        $this->pidList = array();
      }
      else {
        throw new Exception(t("You do not have access to remove bookmarks from the list @bookmarkname", array("@bookmarkname" => $this->bookmarkName)));
      }
    }
    else {
      $superserial = unserialize($_SESSION['islandora_bookmark'][$this->bookmarkId]);
      $superserial->pidList = array();
      $_SESSION['islandora_bookmark'][$this->bookmarkId] = serialize($superserial);
    }
  }

  /**
   * Removes the given users from the object and either the database or session.
   *
   * @global type $user
   *
   * @param array $users
   *   An array of users.
   */
  public function removeUsers($users) {
    global $user;
    $this->userList = array_diff($this->userList, $users);
    if (!$this->bookmarkSession) {
      if ($this->managementAccess()) {
        if (in_array($this->bookmarkOwner, $users)) {
          $this->removeList();
          return;
        }
        foreach ($users as $value) {
          $this->removeUserDB($value);
        }
      }
      elseif (in_array($this->userId, $users)) {
        $this->removeUserDB($this->userId);
      }
      else {
        throw new Exception(t("You do not have access to remove users from the list @bookmarkname", array("@bookmarkname" => $this->bookmarkName)));
      }
    }
    else {
      $superserial = unserialize($_SESSION['islandora_bookmark'][$this->bookmarkId]);

      foreach ($superserial->userList as $key => $value) {
        if (in_array($this->bookmarkOwner, $users)) {
          $this->removeList();
          return;
        }
        if (in_array($value, $users)) {
          unset($superserial->userList[$key]);
          $_SESSION['islandora_bookmark'][$this->bookmarkId] = serialize($superserial);
        }
      }
    }
  }

  /**
   * Adds the specified pids to the object and either the database or session.
   *
   * @param array $pids
   *   An array of pids.
   *
   * @throws Exception
   */
  public function addPids($pids) {
    foreach ($pids as $key => $value) {
      if (islandora_object_load($value)) {
        if (!in_array($value, $this->pidList)) {
          array_push($this->pidList, $value);

          if (!$this->bookmarkSession) {
            if ($this->managementAccess()) {
              $this->addPidDB($value);
            }
            else {
              throw new Exception(t("You do not have access to add bookmarks from the list @bookmarkname", array("@bookmarkname" => $this->bookmarkName)));
            }
          }
          else {
            $superserial = unserialize($_SESSION['islandora_bookmark'][$this->bookmarkId]);
            $superserial->pidList[] = $value;
            $_SESSION['islandora_bookmark'][$this->bookmarkId] = serialize($superserial);
          }
        }
      }
      else {
        throw new Exception(t("Could not bookmark '%value' to '%bookmark' as the object doesn't exist.", array(
              '%value' => $value, '%bookmark' => $this->bookmarkName)));
      }
    }
  }

  /**
   * Adds the specified users to the object and either the database or session.
   *
   * @param array $users
   *   An array of users.
   */
  public function addUsers($users) {
    foreach ($users as $key => $value) {
      if (!in_array($value, $this->userList)) {
        array_push($this->userList, $value);

        if (!$this->bookmarkSession) {
          if ($this->managementAccess()) {
            $this->addUserDB($value);
          }
          else {
            throw new Exception(t("You do not have access to add users from the list @bookmarkname", array("@bookmarkname" => $this->bookmarkName)));
          }
        }
        else {
          $superserial = unserialize($_SESSION['islandora_bookmark'][$this->bookmarkId]);
          $superserial->userList[] = $value;
          $_SESSION['islandora_bookmark'][$this->bookmarkId] = serialize($superserial);
        }
      }
    }
  }

  /**
   * Inserts a record into the database or adds the object to the session.
   */
  public function createList() {
    $this->bookmarkOwner = $this->userId;

    if (!$this->bookmarkSession) {
      $fields = array(
        'listname' => $this->bookmarkName,
        'listowner' => $this->userId,
      );
      $list_id = db_insert('islandora_bookmark_list_names')->fields($fields)->execute();

      $this->bookmarkId = $list_id;

      foreach ($this->userList as $key => $value) {
        $fields = array(
          'uid' => $value,
          'listid' => $this->bookmarkId,
        );
        db_insert('islandora_bookmark_list_users')->fields($fields)->execute();
      }
      foreach ($this->pidList as $key => $value) {
        $fields = array(
          'listid' => $this->bookmarkId,
          'pidid' => $value,
        );
        db_insert('islandora_bookmark_list_pids')->fields($fields)->execute();
      }
    }
    else {
      // Add to session.
      if (isset($_SESSION['islandora_bookmark'])) {
        $index = count($_SESSION['islandora_bookmark']);
      }
      else {
        $index = 0;
      }
      $this->bookmarkId = 'session_' . $index;
      $_SESSION['islandora_bookmark'][$this->bookmarkId] = serialize($this);
    }
  }

  /**
   * Removes the record from the database or the object from the session.
   */
  public function removeList() {
    if (!$this->bookmarkSession) {
      if ($this->managementAccess()) {
        db_delete('islandora_bookmark_list_names')
          ->condition('listid', $this->bookmarkId)
          ->execute();

        foreach ($this->userList as $key => $value) {
          db_delete('islandora_bookmark_list_users')
            ->condition('listid', $this->bookmarkId)
            ->execute();
        }

        foreach ($this->pidList as $key => $value) {
          db_delete('islandora_bookmark_list_pids')
            ->condition('listid', $this->bookmarkId)
            ->execute();
        }
      }
      else {
        throw new Exception(t("You do not have access to remove the list @bookmarkname", array("@bookmarkname" => $this->bookmarkName)));
      }
    }
    else {
      unset($_SESSION['islandora_bookmark'][$this->bookmarkId]);
    }
  }

  /**
   * Removes the specified user from the object's entry in the database.
   *
   * @param int $user_delete
   *   The user to be removed.
   */
  protected function removeUserDB($user_delete) {
    $and = db_and()->condition('uid', $user_delete)->condition('listid', $this->bookmarkId);
    db_delete('islandora_bookmark_list_users')->condition($and)->execute();
  }

  /**
   * Adds the specified user to the object's entry in the database.
   *
   * @param int $user_add
   *   The user to be added.
   */
  protected function addUserDB($user_add) {
    $fields = array(
      'uid' => $user_add,
      'listid' => $this->bookmarkId,
    );
    db_insert('islandora_bookmark_list_users')->fields($fields)->execute();
  }

  /**
   * Removes the specified pid from the object's entry in the database.
   *
   * @param string $pid
   *   The pid to be removed.
   */
  protected function removePidDB($pid) {
    $and = db_and()->condition('pidid', $pid)->condition('listid', $this->bookmarkId);
    db_delete('islandora_bookmark_list_pids')->condition($and)->execute();
  }

  /**
   * Add the specified pid to the object's entry in the database.
   *
   * @param string $pid
   *   The user to be added.
   */
  protected function addPidDB($pid) {
    $fields = array(
      'listid' => $this->bookmarkId,
      'pidid' => $pid,
    );
    db_insert('islandora_bookmark_list_pids')->fields($fields)->execute();
  }

  /**
   * Changes the name of the object in the session or the database.
   *
   * @param string $name
   *   The new name of the object.
   */
  public function changeListName($name) {
    $this->bookmarkName = $name;
    if (!$this->bookmarkSession) {
      db_update('islandora_bookmark_list_names')
        ->fields(array(
            'listname' => $name,
          ))
        ->condition('listid', $this->bookmarkId)
        ->execute();
    }
    else {
      $_SESSION['islandora_bookmark'][$this->bookmarkId] = serialize($this);
    }
  }

  /**
   * Check if user can execute management functions on the current Bookmark.
   *
   * @return bool
   *   TRUE if they can execute management functions, FALSE otherwise.
   */
  public function managementAccess() {
    if ($this->userId === $this->bookmarkOwner) {
      return TRUE;
    }
    else {
      return FALSE;
    }
  }
}
