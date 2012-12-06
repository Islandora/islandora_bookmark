<?php

/**
 * @file
 *
 * Definition of a Bookmark object.
 */

/**
 * Class that defines a Bookmark object. A Bookmark object needs only to be
 * constructed such that it contains a listname and listid for uniqueness.
 *
 */
class Bookmark {
  /**
   * The list of users who belong to this object.
   *
   * @var array
   */
  public $user_list = array();

  /**
   * The list of pids that belong to this object.
   *
   * @var array
   */
  public $pid_list = array();

  /**
   * The name of the object.
   *
   * @var string
   */
  public $bookmark_name;

  /**
   * The unique ID of the object. This is generated through the database or
   * explicity assigned when adding objects to the session.
   *
   * @var integer
   */
  public $bookmark_id;

  /**
   * The owner of the object.
   *
   * @var integer
   */
  public $bookmark_owner;

  /**
   * The user id of the current user accessing the object.
   *
   * @var integer
   */
  public $user_id;

  /**
   * Whether the object is created or retrieved through the session.
   *
   * @var boolean
   */
  public $bookmark_session = FALSE;

  /**
   * Constructor for the Bookmark object.
   *
   * @param array $params
   */
  public function __construct($params = array()) {
    $this->user_id = $params['user_id'];
    $this->user_list = $params['bookmark_users'];

    if (!empty($params['bookmark_session'])) {
      $this->bookmark_session = $params['bookmark_session'];
    }

    if (!empty($params['bookmark_pids'])) {
      $this->pid_list = $params['bookmark_pids'];
    }

    if (!empty($params['bookmark_name'])) {
      $this->bookmark_name = $params['bookmark_name'];
    }

    if (!empty($params['bookmark_id'])) {
      $this->bookmark_id = $params['bookmark_id'];
    }

    if (!$params['retrieve']) {
      $this->create_list();
    }
    else {
    $this->bookmark_owner = $params['bookmark_owner'];
    }
  }

  /**
   * Removes the specified pids from the object and either
   * the database or session.
   *
   * @param array $pids
   *   An array of pids.
   */
  public function remove_pids($pids) {
    $this->pid_list = array_diff($this->pid_list, $pids);

    if (!$this->bookmark_session) {
      if ($this->management_access()) {
        foreach ($pids as $value) {
          $this->remove_pid_db($value);
        }
      }
      else {
        throw new Exception(t("You do not have access to remove pids from the list $this->bookmark_name"));
      }
    }
    else {
      $superserial = unserialize($_SESSION['islandora_bookmark'][$this->bookmark_id]);

      foreach ($superserial->pid_list as $key => $value) {
        if (in_array($value, $pids)) {
          unset($superserial->pid_list[$key]);
          $_SESSION['islandora_bookmark'][$this->bookmark_id] = serialize($superserial);
        }
      }
    }
  }

  /**
   * Removes the specified users from the object and either the database
   * or session.
   *
   * @global type $user
   *
   * @param array $users
   *   An array of users.
   */
  public function remove_users($users) {
    global $user;
    $this->user_list = array_diff($this->user_list, $users);
    if (!$this->bookmark_session) {
      if ($this->management_access()) {
        if (in_array($this->bookmark_owner, $users)) {
          $this->remove_list();
          return;
        }
        foreach ($users as $value) {
          $this->remove_user_db($value);
        }
      }
      elseif (in_array($this->user_id, $users)) {
        $this->remove_user_db($this->user_id);
      }
      else {
        throw new Exception(t("You do not have access to remove users from the list $this->bookmark_name"));
      }
    }
    else {
      $superserial = unserialize($_SESSION['islandora_bookmark'][$this->bookmark_id]);

      foreach ($superserial->user_list as $key => $value) {
        if (in_array($this->bookmark_owner, $users)) {
          $this->remove_list();
          return;
        }
        if (in_array($value, $users)) {
          unset($superserial->user_list[$key]);
          $_SESSION['islandora_bookmark'][$this->bookmark_id] = serialize($superserial);
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
   *
   */
  public function add_pids($pids) {
    module_load_include('inc', 'fedora_repository', 'api/tuque');

    foreach ($pids as $key => $value) {
      if (islandora_object_load($value)) {
          if (!in_array($value, $this->pid_list)) {
          array_push($this->pid_list, $value);

            if (!$this->bookmark_session) {
              if ($this->management_access()) {
                $this->add_pid_db($value);
              }
              else {
                throw new Exception(t("You do not have access to add pids to the list $this->bookmark_name"));
              }
            }
            else {
            $superserial = unserialize($_SESSION['islandora_bookmark'][$this->bookmark_id]);
            $superserial->pid_list[] = $value;
            $_SESSION['islandora_bookmark'][$this->bookmark_id] = serialize($superserial);
            }
          }
      }
      else {
        throw new Exception(t("Could not add the pid '" . $value . "' to '" . $this->bookmark_name . "' as the pid doesn't exist!"));
      }
    }
  }

  /**
   * Adds the specified users to the object and either the database or session.
   *
   * @param type $users
   *   An array of users.
   */
  public function add_users($users) {
    foreach ($users as $key => $value) {
      if (!in_array($value, $this->user_list)) {
        array_push($this->user_list, $value);

          if (!$this->bookmark_session) {
            if ($this->management_access()) {
              $this->add_user_db($value);
            }
            else {
              throw new Exception(t("You do not have access to add users to the list $this->bookmark_name"));
            }
          }
          else {
            $superserial = unserialize($_SESSION['islandora_bookmark'][$this->bookmark_id]);
            $superserial->user_list[] = $value;
            $_SESSION['islandora_bookmark'][$this->bookmark_id] = serialize($superserial);
        }
      }
    }
  }

  /**
   * Inserts a record into the database or adds the object to the session.
   */
  public function create_list() {
    $this->bookmark_owner = $this->user_id;

    if (!$this->bookmark_session) {
      $fields = array(
        'listname' => $this->bookmark_name,
        'listowner' => $this->user_id,
      );
      $list_id = db_insert('islandora_bookmark_list_names')->fields($fields)->execute();

      $this->bookmark_id = $list_id;

      foreach ($this->user_list as $key => $value) {
        $fields = array(
          'uid' => $value,
          'listid' => $this->bookmark_id,
        );
        db_insert('islandora_bookmark_list_users')->fields($fields)->execute();
      }
      foreach ($this->pid_list as $key => $value) {
        $fields = array(
          'listid' => $this->bookmark_id,
          'pidid' => $value,
        );
        db_insert('islandora_bookmark_list_pids')->fields($fields)->execute();
      }
    }
    else {
      // Add to session
      if (isset($_SESSION['islandora_bookmark'])) {
        $index = count($_SESSION['islandora_bookmark']);
      }
      else {
        $index = 0;
      }
      $this->bookmark_id = 'session_' . $index;
      $_SESSION['islandora_bookmark'][$this->bookmark_id] = serialize($this);
    }
  }

  /**
   * Removes the record from the database or the object from the session.
   */
  public function remove_list() {
    if (!$this->bookmark_session) {
      if ($this->management_access()) {
        db_delete('islandora_bookmark_list_names')
        ->condition('listid', $this->bookmark_id)
        ->execute();

        foreach ($this->user_list as $key => $value) {
          db_delete('islandora_bookmark_list_users')
          ->condition('listid', $this->bookmark_id)
          ->execute();
        }

        foreach ($this->pid_list as $key => $value) {
          db_delete('islandora_bookmark_list_pids')
          ->condition('listid', $this->bookmark_id)
          ->execute();
        }
      }
      else {
        throw new Exception(t("You do not have access to remove the list $this->bookmark_name"));
      }
    }
    else {
      unset($_SESSION['islandora_bookmark'][$this->bookmark_id]);
    }
  }

  /**
   * Removes the specified user from the object's entry in the database.
   *
   * @param integer $userdelete
   *   The user to be removed.
   */
  private function remove_user_db($user_delete) {
    $and = db_and()->condition('uid', $user_delete)->condition('listid', $this->bookmark_id);
    db_delete('islandora_bookmark_list_users')->condition($and)->execute();
  }

  /**
   * Adds the specified user to the object's entry in the database.
   *
   * @param integer $useradd
   *   The user to be added.
   */
  private function add_user_db($user_add) {
    $fields = array(
      'uid' => $user_add,
      'listid' => $this->bookmark_id,
    );
    db_insert('islandora_bookmark_list_users')->fields($fields)->execute();
  }

  /**
   * Removes the specified pid from the object's entry in the database.
   *
   * @param string $pid
   *   The pid to be removed.
   */
  private function remove_pid_db($pid) {
    $and = db_and()->condition('pidid', $pid)->condition('listid', $this->bookmark_id);
    db_delete('islandora_bookmark_list_pids')->condition($and)->execute();
  }

  /**
   * Add the specified pid to the object's entry in the database.
   *
   * @param string $pid
   *   The user to be added.
   */
  private function add_pid_db($pid) {
    $fields = array(
      'listid' => $this->bookmark_id,
      'pidid' => $pid,
    );
    db_insert('islandora_bookmark_list_pids')->fields($fields)->execute();
  }

  /**
   * Changes the name of the object in the session or updates the object's
   * entry in the database
   *
   * @param string $name
   *   The new name of the object.
   */
  public function change_list_name($name) {
    $this->bookmark_name = $name;
    if (!$this->bookmark_session) {
      db_update('islandora_bookmark_list_names')
        ->fields(array(
            'listname' => $name,
        ))
        ->condition('listid', $this->bookmark_id)
        ->execute();
    }
    else {
      $_SESSION['islandora_bookmark'][$this->bookmark_id] = serialize($this);
    }
  }

  /**
   * Function to check whether the current user can execute management functions
   * on the current Bookmark.
   *
   * @return boolean
   */
  public function management_access() {
    if ($this->user_id === $this->bookmark_owner) {
      return TRUE;
    }
    else {
      return FALSE;
    }
  }
}


