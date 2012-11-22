<?php

/**
 * @file
 * Houses helper functions used in the islandora_pid_list module
 * 
 */

/**
 * Retrieves lists that the specified user has access to.
 * 
 * @global type $user
 *
 * @param array $params
 *   Which value to search.
 *
 * @return array
 *   An array of PidList objects.
 */

function get_user_owned_bookmarks() {
  module_load_include('inc', 'islandora_bookmark', 'Bookmark');
  global $user;
  
  $listids = array();
  $pidlist = array();
    
  $list_query = db_query('SELECT listid FROM {islandora_bookmark_list_names} WHERE listowner = :uid ORDER BY listid', array(':uid' => $user->uid));
 
  foreach ($list_query as $record) {
    if (!in_array($record->listid, $listids)) {
      $listids[] = $record->listid;
    }
  }
  
  foreach ($listids as $key => $value) {
    $listpids = array();
    $listusers = array();
    
    $pid_query = db_query('SELECT pidid FROM {islandora_bookmark_list_pids} WHERE listid = :listid', array(':listid' => $value));
    
    foreach ($pid_query as $record) {
      $listpids[] = $record->pidid;
    }
             
    $user_query = db_query('SELECT uid FROM {islandora_bookmark_list_users} WHERE listid = :listid', array(':listid' => $value));
    
    foreach ($user_query as $record) {
      $listusers[] = $record->uid;
    }
        
    $list_record = db_query('SELECT listname, listowner from {islandora_bookmark_list_names} WHERE listid = :listid', array(':listid' => $value))->fetchObject();
    $listid = $value;

    $params = array(
      'retrieve' => TRUE,
      'user_id' => $user->uid,
      'bookmark_owner' => $list_record->listowner,
      'bookmark_users' => $listusers,
      'bookmark_pids' => $listpids,
      'bookmark_name' => $list_record->listname,
      'bookmark_id' => $listid
    );
    $templist = new Bookmark($params);

    $pidlist[] = $templist;
   }
         
   // User is anon, use session as well
   if ($user->uid == 0 && !empty($_SESSION['islandora_bookmark'])) {
     foreach ($_SESSION['islandora_bookmark'] as $value) {
       if (isset($pid)) {
         $templist = unserialize($value);
           if (in_array($pid, $templist->pid_list)) {
             $pidlist[] = $templist;
           }
       }
       else {
          $pidlist[] = unserialize($value);
        }
     }
   }
  return $pidlist;
}

function get_user_shared_bookmarks() {
  module_load_include('inc', 'islandora_bookmark', 'Bookmark');
  global $user;
  
  $owner_ids = array();
  $shared_ids = array();
  $pidlist = array();
  
    
  $owner_query = db_query('SELECT listid FROM {islandora_bookmark_list_names} WHERE listowner = :uid ORDER BY listid', array(':uid' => $user->uid));
  
  foreach ($owner_query as $record) {
   $owner_ids[] = $record->listid;
  }
  $shared_query = db_query('SELECT listid FROM {islandora_bookmark_list_users} WHERE uid = :uid ORDER BY listid', array(':uid' => $user->uid));
  
  foreach ($shared_query as $record) {
    $shared_ids[] = $record->listid;
  }
  $shared_ids = array_diff($shared_ids, $owner_ids);
  
  foreach ($shared_ids as $key => $value) {
    
    $listpids = array();
    $listusers = array();
    
    $pid_query = db_query('SELECT pidid FROM {islandora_bookmark_list_pids} WHERE listid = :listid', array(':listid' => $value));
    
    foreach ($pid_query as $record) {
      $listpids[] = $record->pidid;
    }
             
    $user_query = db_query('SELECT uid FROM {islandora_bookmark_list_users} WHERE listid = :listid', array(':listid' => $value));
    
    foreach ($user_query as $record) {
      $listusers[] = $record->uid;
    }
        
    $list_record = db_query('SELECT listname, listowner from {islandora_bookmark_list_names} WHERE listid = :listid', array(':listid' => $value))->fetchObject();
    $listid = $value;

    $params = array(
      'retrieve' => TRUE,
      'user_id' => $user->uid,
      'bookmark_owner' => $list_record->listowner,
      'bookmark_users' => $listusers,
      'bookmark_pids' => $listpids,
      'bookmark_name' => $list_record->listname,
      'bookmark_id' => $listid
    );
    $templist = new Bookmark($params);
    $pidlist[] = $templist;
   }
   return $pidlist;
}

/**
 * Retrieves a list given a specified ID.
 * 
 * @global type $user
 * 
 * @param integer $listid
 *   List ID to be searched for.
 * 
 * @return PidList
 *   PidList object.
 */
function get_bookmark_by_number($listid) {
  module_load_include('inc', 'islandora_bookmark', 'Bookmark');
  global $user;
   
  if (!preg_match('/session_/', $listid)) {
    $listresult = db_query('SELECT listname, listowner from {islandora_bookmark_list_names} WHERE listid = :listid', array(':listid' => $listid))->fetchObject();
        
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
    $templist = unserialize($_SESSION['islandora_bookmark'][$listid]);
  }
  return $templist;
}

function get_pid_list_by_number($listid) {
  module_load_include('inc', 'islandora_bookmark', 'Bookmark');
  global $user;
   
  if (!preg_match('/session_/', $listid)) {
    $listresult = db_query('SELECT listname, listowner from {islandora_bookmark_list_names} WHERE listid = :listid', array(':listid' => $listid))->fetchObject();
        
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
    $templist = unserialize($_SESSION['islandora_pid_list'][$listid]);
  }
  return $templist;
}

