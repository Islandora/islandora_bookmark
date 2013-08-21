<?php

/**
 * @file
 * Houses helper functions used in the islandora_bookmark module
 */

/**
 * Retrieves lists that the specified user has access to.
 *
 * @global type $user
 *
 * @return array
 *   An array of PidList objects.
 */
function islandora_bookmark_get_user_owned_bookmarks() {
  global $user;

  $listids = array();
  $bookmark_list = array();

  $list_query = db_query('SELECT listid FROM {islandora_bookmark_list_names} WHERE listowner = :uid ORDER BY listid', array(':uid' => $user->uid));

  foreach ($list_query as $record) {
    if (!in_array($record->listid, $listids)) {
      $listids[] = $record->listid;
    }
  }

  foreach ($listids as $key => $value) {

    $listpids = islandora_bookmark_list_pids_query($value);

    $listusers = islandora_bookmark_list_users_query($value);

    $list_record = db_query('SELECT listname, listowner from {islandora_bookmark_list_names} WHERE listid = :listid', array(':listid' => $value))->fetchObject();
    $listid = $value;

    $params = array(
      'retrieve' => TRUE,
      'user_id' => $user->uid,
      'bookmark_owner' => $list_record->listowner,
      'bookmark_users' => $listusers,
      'bookmark_pids' => $listpids,
      'bookmark_name' => $list_record->listname,
      'bookmark_id' => $listid,
    );
    $templist = new Bookmark($params);

    $bookmark_list[] = $templist;
  }

  // User is anon, use session as well.
  if ($user->uid == 0 && !empty($_SESSION['islandora_bookmark'])) {
    foreach ($_SESSION['islandora_bookmark'] as $value) {
      if (isset($pid)) {
        $templist = unserialize($value);
        if (in_array($pid, $templist->pidList)) {
          $bookmark_list[] = $templist;
        }
      }
      else {
        $bookmark_list[] = unserialize($value);
      }
    }
  }
  return $bookmark_list;
}

/**
 * Retrieves all Bookmarks the current user is shared on.
 *
 * @global type $user
 * @return array
 *   An array of Bookmarks.
 */
function islandora_bookmark_get_user_shared_bookmarks() {
  global $user;

  $owner_ids = array();
  $shared_ids = array();
  $bookmark_list = array();

  $owner_query = db_query('SELECT listid FROM {islandora_bookmark_list_names} WHERE listowner = :uid ORDER BY listid', array(':uid' => $user->uid));

  foreach ($owner_query as $record) {
    $owner_ids[] = $record->listid;
  }
  $shared_query = db_query('SELECT listid FROM {islandora_bookmark_list_users} WHERE uid = :uid OR uid = 0 ORDER BY listid', array(':uid' => $user->uid));

  foreach ($shared_query as $record) {
    $shared_ids[] = $record->listid;
  }
  $shared_ids = array_diff($shared_ids, $owner_ids);

  foreach ($shared_ids as $key => $value) {

    $listpids = islandora_bookmark_list_pids_query($value);

    $listusers = islandora_bookmark_list_users_query($value);

    $list_record = db_query('SELECT listname, listowner from {islandora_bookmark_list_names} WHERE listid = :listid', array(':listid' => $value))->fetchObject();
    $listid = $value;

    $params = array(
      'retrieve' => TRUE,
      'user_id' => $user->uid,
      'bookmark_owner' => $list_record->listowner,
      'bookmark_users' => $listusers,
      'bookmark_pids' => $listpids,
      'bookmark_name' => $list_record->listname,
      'bookmark_id' => $listid,
    );
    $templist = new Bookmark($params);
    $bookmark_list[] = $templist;
  }
  return $bookmark_list;
}

/**
 * Retrieves a Bookmark given a specified ID.
 *
 * @global type $user
 *
 * @param int $listid
 *   List ID to be searched for.
 *
 * @return bool|PidList
 *   PidList object or FALSE if the list does not exist.
 */
function islandora_bookmark_get_bookmark_by_number($listid) {
  global $user;

  if (!preg_match('/session_/', $listid)) {
    $listresult = db_query('SELECT listname, listowner from {islandora_bookmark_list_names} WHERE listid = :listid', array(':listid' => $listid))->fetchObject();
    if ($listresult) {
      $userresult = db_query('SELECT uid from {islandora_bookmark_list_users} WHERE listid = :listid', array(':listid' => $listid));

      $userlist = array();

      foreach ($userresult as $record) {
        array_push($userlist, $record->uid);
      }

      $pidresult = db_query('SELECT pidid from {islandora_bookmark_list_pids} WHERE listid = :listid', array(':listid' => $listid));
      $pidlist = array();

      foreach ($pidresult as $record) {
        array_push($pidlist, $record->pidid);
      }

      $params = array(
        'retrieve' => TRUE,
        'user_id' => $user->uid,
        'bookmark_owner' => $listresult->listowner,
        'bookmark_name' => $listresult->listname,
        'bookmark_id' => $listid,
        'bookmark_users' => $userlist,
        'bookmark_pids' => $pidlist,
      );
      $templist = new Bookmark($params);
    }
    else {
      return FALSE;
    }
  }
  else {
    $templist = unserialize($_SESSION['islandora_bookmark'][$listid]);
  }
  return $templist;
}

/**
 * Retrieves all Bookmarks visible to the current user.
 *
 * @param string $pid
 *   The object's PID.
 * @param int $uid
 *   Drupal user id.
 *
 * @return array
 *   An array containing listids.
 */
function islandora_bookmark_get_bookmarks_visible_to_user($pid, $uid) {
  $lists = array();
  // SESSION FIRST.
  if ($uid == 0 && !empty($_SESSION['islandora_bookmark'])) {
    foreach ($_SESSION['islandora_bookmark'] as $value) {
      $templist = unserialize($value);
      if (in_array($pid, $templist->pidList)) {
        $lists[$templist->bookmarkId] = $templist->bookmarkId;
      }
    }
  }
  else {
    // DB.
    $list_result = db_query('SELECT u.listid from {islandora_bookmark_list_users} as u INNER JOIN {islandora_bookmark_list_pids} as p ON u.listid = p.listid WHERE uid = :uid AND pidid = :pid', array(':uid' => $uid, ':pid' => $pid));
    foreach ($list_result as $record) {
      $lists[$record->listid] = $record->listid;
    }
  }
  return $lists;
}

/**
 * Retrieves all the pids from the bookmark list id.
 *
 * @param string $value
 *   The listid.
 *
 * @return array
 *   An array containing listpids.
 */
function islandora_bookmark_list_pids_query($value) {
  $pid_query = db_query('SELECT pidid FROM {islandora_bookmark_list_pids} WHERE listid = :listid', array(':listid' => $value));
  $listpids = array();
  foreach ($pid_query as $record) {
    $listpids[] = $record->pidid;
  }
  return $listpids;
}

/**
 * Retrieves all the uids from the bookmark listid.
 *
 * @param string $value
 *   The listid.
 *
 * @return array
 *   An array containing user ids.
 */
function islandora_bookmark_list_users_query($value) {
  $user_query = db_query('SELECT uid FROM {islandora_bookmark_list_users} WHERE listid = :listid', array(':listid' => $value));
  $listusers = array();
  foreach ($user_query as $record) {
    $listusers[] = $record->uid;
  }
  return $listusers;
}
