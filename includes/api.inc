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

  $list_query = db_query('SELECT DISTINCT listid FROM {islandora_bookmark_list_names} WHERE listowner = :uid ORDER BY listid', array(':uid' => $user->uid));
  $listids = (array) $list_query->fetchAll(PDO::FETCH_COLUMN);

  $bookmark_list = array();
  foreach ($listids as $list_id) {
    $templist = BookmarkDatabase::getList($list_id);

    $bookmark_list[] = $templist;
  }

  // User is anon, use session as well.
  if (isset($_SESSION) && $user->uid == 0 && !empty($_SESSION['islandora_bookmark'])) {
    foreach (array_keys($_SESSION['islandora_bookmark']) as $id) {
      $bookmark_list[] = BookmarkSession::getList($id);
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

  foreach ($shared_ids as $id) {
    $templist = Bookmark::getList($id);
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
  $list = Bookmark::getList($listid);
  if ($list === NULL) {
    return FALSE;
  }
  return $list;
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
      $templist = $value;
      if (in_array($pid, $templist->getPids())) {
        $lists[$templist->getId()] = $templist->getId();
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
  $list = Bookmark::getList($value);
  return $list->getPids();
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
  $list = Bookmark::getList($value);
  return $list->getUsers();
}
