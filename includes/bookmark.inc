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
   * Get a list of all the users who a list is shared with.
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
   * @var int
   */
  public $bookmarkOwner;

  /**
   * The ID of the given list.
   *
   * @var mixed
   */
  protected $bookmarkId;

  /**
   * Flag whether or not the given backend is persistent.
   *
   * @var bool
   */
  protected $isPersistent;

  /**
   * Constructor for the Bookmark object.
   */
  public function __construct($id, $name, $owner) {
    $this->bookmarkId = $id;
    $this->bookmarkName = $name;
    $this->bookmarkOwner = $owner;
  }

  /**
   * Somewhat more graceful deprecation of old properties.
   */
  public function __get($property) {
    if ($property == 'userList') {
      module_load_include('inc', 'islandora', 'includes/utilities');
      $message = islandora_deprecated('7.x-1.3', 'Use the methods {get,add,remove}Users(), instead of accessing as a property.');
      trigger_error(filter_xss($message), E_USER_DEPRECATED);
      return $this->getUsers();
    }
    elseif ($property == 'pidList') {
      module_load_include('inc', 'islandora', 'includes/utilities');
      $message = islandora_deprecated('7.x-1.3', 'Use the methods {get,add,remove}Pids(), instead of accessing as a property.');
      trigger_error(filter_xss($message), E_USER_DEPRECATED);
      return $this->getPids();
    }
  }

  /**
   * Somewhat more graceful deprecation of old properties.
   */
  public function __set($property, $value) {
    if ($property == 'userList') {
      module_load_include('inc', 'islandora', 'includes/utilities');
      $message = islandora_deprecated('7.x-1.3', 'Use the methods {get,add,remove}Users(), instead of accessing as a property.');
      trigger_error(filter_xss($message), E_USER_DEPRECATED);
      $this->removeUsers($this->getUsers());
      $this->addUsers($value);
      return $value;
    }
    elseif ($property == 'pidList') {
      module_load_include('inc', 'islandora', 'includes/utilities');
      $message = islandora_deprecated('7.x-1.3', 'Use the methods {get,add,remove}Pids(), instead of accessing as a property.');
      trigger_error(filter_xss($message), E_USER_DEPRECATED);
      $this->removeAllPids();
      $this->addPids($value);
      return $value;
    }
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
   * Accessor for the persistence.
   *
   * @return mixed
   *   The value of $this->isPersistent.
   */
  public function getIsPersistent() {
    return $this->isPersistent;
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

  /**
   * Form building method.
   */
  public function formGet($form, &$form_state) {
    global $user;
    if (module_exists('citation_exporter')) {
      if (CitationExporter::ReadyToExport()) {
        $citation_export_head = array(
          '#type' => 'html_tag',
          '#tag' => 'meta',
          '#attributes' => array(
            'http-equiv' => 'refresh',
            'content' => '1.0;' . url(CITATION_EXPORTER_EXPORT_MENU, array('absolute' => TRUE)),
          ),
        );
        drupal_add_html_head($citation_export_head, 'citation_exporter_head');
      }
    }

    $form = array(
      '#tree' => TRUE,
      '#attached' => array(
        'css' => array(
          drupal_get_path('module', 'islandora_bookmark') . '/css/islandora_bookmark.css',
        ),
      ),
      '#prefix' => '<div id="islandora-bookmark-detailed-overview">',
      '#suffix' => '</div>',
      'bookmarks' => array(
        '#weight' => 0,
        'fieldset' => array(
          '#type' => 'fieldset',
          '#title' => 'Detailed List',
          '#prefix' => '<div id="islandora-bookmark-detailed-list">',
          '#suffix' => '</div>',
        ),
      ),
    );

    // This is the RSS icon link.
    if ($this->getIsPersistent() && $this->getPidCount() > 0) {
      $id = $this->getId();
      $form['bookmark_rss'] = array(
        '#weight' => -1,
        '#markup' => l(theme('image', array(
          'path' => drupal_get_path('module', 'islandora_bookmark') . '/images/rss.png',
          'title' => t('Present Bookmark list in RSS form'),
          'attributes' => array(),
        )), "islandora-bookmark/listid/$id/rss", array('html' => TRUE)),
        '#prefix' => '<div id="islandora-bookmark-rss-format">',
        '#suffix' => '</div>',
      );
    }

    $rows = array();

    // Do pager calcuations to limit the number of objects displayed per page.
    $num_eles = variable_get('islandora_bookmark_detailed_page_elements', 10);
    $num_pids = $this->getPidCount();
    $pager_element = 0;

    $page = pager_default_initialize($num_pids, $num_eles, $pager_element);
    $offset = $page * $num_eles;

    $pids = $this->getPids($offset, $num_eles);

    // Loop the pids and determine how to display the markup.
    foreach ($pids as $pid) {
      $rows[$pid] = array(
        'markup' => islandora_bookmark_generate_markup($pid),
      );
    }
    $row_count = count($rows);

    $header = array('markup' => array('data' => t('Label')));

    $form_state['islandora_bookmark_current_page'] = $page;
    $form_state['islandora_bookmark_pids_on_page'] = $row_count;
    if ($this->managementAccess()) {
      $header['remove'] = array('data' => t('Operations'));
      foreach ($rows as $pid => &$row) {
        $remove_button = array(
          '#type' => 'submit',
          '#name' => 'remove_pid_' . $pid,
          '#id' => 'remove_pid_' . $pid,
          '#button_type' => 'submit',
          '#pid' => $pid,
          '#value' => t('Remove'),

        );
        $row['remove'] = array(
          'data' => drupal_render($remove_button),
          'class' => 'operations',
        );
      }
      // XXX: Silly looping with references...
      unset($row);
    }

    $form['bookmarks']['fieldset'] += array(
      'table' => array(
        '#header' => $header,
        '#type' => 'tableselect',
        '#options' => $rows,
        '#empty' => t('@type list is empty.', array(
          '@type' => ucwords(variable_get('islandora_bookmark_type', 'bookmark')),
        )),
        '#weight' => -2,
      ),
      'pager' => array(
        '#markup' => theme('pager', array(
          'element' => $pager_element,
        )),
        '#weight' => -1,
      ),
    );
    if ($row_count > 0) {
      if ($this->managementAccess()) {
        $form['bookmarks']['fieldset'] += array(
          'remove_select' => array(
            '#type' => 'submit',
            '#value' => t('Remove selected'),
            '#name' => 'islandora_bookmark_remove_selected',
          ),
          'remove_all' => array(
            '#type' => 'submit',
            '#value' => t('Remove all'),
            '#name' => 'islandora_bookmark_remove_all',
          ),
        );
      }

      if (user_access('export islandora bookmarks')) {
        $export_styles = array();
        $export = islandora_bookmark_get_export_options();
        $export_options['default'] = t('- Export type -');

        foreach ($export as $module_name => $export_array) {
          foreach ($export_array as $name => $export_func) {
            $export_options[t("@module_name", array("@module_name" => $module_name))][$name] = $name;
            $form_state['islandora_bookmark_export'][$name] = $export_func;
            $export_styles[$name] = islandora_bookmark_get_export_format_options($name);
          }
        }

        $style_elements = array();
        foreach ($export_styles as $key => $value) {
          if (!empty($value)) {
            $style_elements[$key] = array(
              '#type' => 'select',
              '#title' => t('Style'),
              '#options' => $value,
              '#weight' => 4,
              '#name' => 'islandora_bookmark_export_styles_' . $key,
              '#states' => array(
                'visible' => array(
                  ':input[name="islandora_bookmark_export_options_select"]' => array('value' => $key),
                ),
              ),
            );
          }
        }
        $form['bookmarks']['fieldset'] += array(
          'export_title' => array(
            '#markup' => '<h3>' . t('Export') . '</h3>',
            '#weight' => 3,
          ),
          'export_options' => array(
            '#type' => 'select',
            '#title' => t('Format'),
            '#options' => $export_options,
            '#weight' => 4,
            '#name' => 'islandora_bookmark_export_options_select',
          ),
          'format_opts' => array(
            '#weight' => 4,
            'export_styles' => $style_elements,
            'export_selected_submit' => array(
              '#type' => 'submit',
              '#value' => t('Export selected @type', array(
                // XXX: Should be made better for translating... (Not all
                // languages add an "s" to pluralize).
                '@type' => variable_get('islandora_bookmark_type', 'bookmark') . 's',
              )),
              '#name' => 'export_selected',
              '#weight' => 5,
            ),
            'export_all_submit' => array(
              '#type' => 'submit',
              '#value' => t('Export all @type', array(
                // XXX: Should be made better for translating... (Not all
                // languages add an "s" to pluralize).
                '@type' => variable_get('islandora_bookmark_type', 'bookmark') . 's',
              )),
              '#name' => 'export_all',
              '#weight' => 6,
            ),
          ),
        );
      }
    }

    // Share.
    if ($this->managementAccess()) {
      if (user_access('share islandora bookmarks')) {
        // Triggered element (Add item or remove).
        if (isset($form_state['triggering_element'])) {
          // Add.
          if ($form_state['triggering_element']['#button_id'] == 'islandora-bookmark-add-user') {
            $add_user = $form_state['input']['bookmarks']['users']['listusers'];
            if (drupal_strlen($add_user)) {
              if ($add_user == 0) {
                $output_user = t('Everyone');
              }
              else {
                $output_user = user_load($add_user)->name;
              }
              drupal_set_message(t('The user @adduser has been added to @listname', array(
                '@listname' => $this->bookmarkName,
                '@adduser' => $output_user,
              )));
              $this->addUsers(array($add_user));
            }
          }
          // Remove.
          if ($form_state['triggering_element']['#button_id'] == 'islandora-bookmark-remove-user') {
            $row = $form_state['triggering_element']['#row'];
            $remove_user = $form_state['values']['bookmarks']['users'][$row]['user'];
            if ($remove_user == 0) {
              $output_user = t('Everyone');
            }
            else {
              $output_user = user_load($remove_user)->name;
            }
            drupal_set_message(t('The user @username has been removed from the list @listname.',
                array(
                  '@listname' => $this->bookmarkName,
                  '@username' => $output_user,
                )
              ));
            $this->removeUsers(array($remove_user));
          }
        }

        $fku = array();
        if ($this->getIsPersistent()) {

          $users = $this->getUsers();
          if (count($users) === 0) {
            $fku['empty'] = array(
              '#type' => 'item',
              '#markup' => t('@type list is not shared with other users.', array(
                '@type' => ucwords(variable_get('islandora_bookmark_type', 'bookmark')),
              )),
            );
          }
          else {
            $fku['title'] = array(
              '#markup' => '<h3>' . t('Shared with') . ':</h3>',
            );
            // Remove the user from the populated shared users.
            $bookmark_users = $this->getUsers();
            $key = array_search($user->uid, $bookmark_users);
            if ($key !== FALSE) {
              unset($bookmark_users[$key]);
            }

            foreach ($bookmark_users as $itr => $uid) {
              $tempuser = user_load($uid);
              if ($tempuser->uid == 0) {
                $user_output = t('Everyone');
              }
              else {
                $user_output = $tempuser->name;
              }
              if ($tempuser->uid != $user->uid) {
                $fku[$itr] = array(
                  '#prefix' => '<div class="islandora-bookmark-users">',
                  '#suffix' => '</div>',
                  'label' => array(
                    '#type' => 'item',
                    '#markup' => $user_output,
                  ),
                  'user' => array(
                    '#type' => 'value',
                    '#value' => $uid,
                  ),
                  'list-id' => array(
                    '#type' => 'value',
                    '#value' => $this->getId(),
                  ),
                  'remove' => array(
                    '#type' => 'button',
                    '#value' => t('Remove'),
                    '#name' => 'islandora_bookmark_remove_user_' . $itr,
                    '#row' => $itr,
                    '#button_id' => 'islandora-bookmark-remove-user',
                    '#ajax' => array(
                      'event' => 'click',
                      'callback' => 'islandora_bookmark_remove_user',
                      'wrapper' => 'islandora-bookmark-detailed-overview-users',
                      'method' => 'replace',
                    ),
                  ),
                );
              }
            }
          }

          $selectoptions = array(
            '' => t('- Select user -'),
          );
          if ($user->uid != 0) {
            $options['0'] = array(
              'uid' => 0,
              'name' => t('Everyone'),
            );
          }
          // Get all users for use in select for forms.
          $result = db_select('users', 'u')
            ->fields('u', array('uid', 'name'))
            ->condition('STATUS', 0, '!=')
            ->condition('uid', $user->uid, '!=')
            ->orderBy('uid')
            ->execute();

          $options += $result->fetchAllAssoc('uid', PDO::FETCH_ASSOC);
          if (isset($options['0'])) {
            $options[0]['name'] = t('Everyone');
          }

          $available = array_diff_key($options, drupal_map_assoc($this->getUsers()));
          foreach ($available as $index => $item) {
            $selectoptions[$index] = $item['uid'] . ': ' . $item['name'];
          }
          $fku['listusers'] = array(
            '#type' => 'select',
            '#default value' => t("- Select user -"),
            '#options' => $selectoptions,
            '#width' => 20,
            '#prefix' => '<div class="islandora-bookmark-adduser">',
          );
          $fku['listadd'] = array(
            '#type' => 'button',
            '#value' => t('Share'),
            '#button_id' => 'islandora-bookmark-add-user',
            '#suffix' => '</div>',
            '#ajax' => array(
              'event' => 'click',
              'callback' => 'islandora_bookmark_add_user',
              'wrapper' => 'islandora-bookmark-detailed-overview-users',
              'method' => 'replace',
            ),
          );

          $form['bookmarks']['users'] = array(
            '#type' => 'fieldset',
            '#collapsible' => TRUE,
            '#collapsed' => FALSE,
            '#title' => t('Share'),
            '#prefix' => '<div id="islandora-bookmark-detailed-overview-users">',
            '#suffix' => '</div>',
          ) + $fku;
        }
      }
    }
    else {
      if ($user->uid != 0) {
        $form['bookmarks']['remove_self'] = array(
          '#type' => 'submit',
          '#value' => t('Remove yourself from this list'),
          '#submit' => array('islandora_bookmark_remove_self'),
        );
      }
    }

    return $form;
  }

  /**
   * Form validation handler.
   */
  public function formValidate(&$form, &$form_state) {
    if (isset($form_state['triggering_element']['#button_id'])) {
      if ($form_state['triggering_element']['#button_id'] == 'islandora-bookmark-add-user') {
        $parents = array('bookmarks', 'users', 'listusers');
        $add_user = drupal_array_get_nested_value($form_state['values'], $parents);
        if (drupal_strlen($add_user) === 0) {
          form_set_error(implode('][', $parents), t('Please select a user to add to @listname.', array(
            '@listname' => $this->bookmarkName,
          )));
        }
      }
    }
  }

  /**
   * Form submission handler.
   */
  public function formSubmit(&$form, &$form_state) {
    foreach ($_POST as $key => $value) {
      preg_match('/remove_pid_(.*)/', $key, $match);
      if ($match) {
        break;
      }
    }
    if ($match) {
      $this->removePids(array($match[1]));
      $removed_object = islandora_object_load($match[1]);

      drupal_set_message(t('The @type @objectname has been removed from @listname.', array(
        '@type' => variable_get('islandora_bookmark_type', 'bookmark'),
        '@listname' => $this->bookmarkName,
        '@objectname' => $removed_object->label,
      )));
      $current_page = $form_state['islandora_bookmark_current_page'];
      $current_pids = $form_state['islandora_bookmark_pids_on_page'];

      if (($current_pids - 1) != 0) {
        $query = array(
          'query' => array(
            'page' => $current_page,
          ),
        );
        $destination = drupal_parse_url('islandora-bookmark/listid/' . $this->getId(), array('query' => $query));
        drupal_goto($destination['path'], $query);
      }
      elseif ($current_page == 0) {
        $destination = drupal_parse_url('islandora-bookmark/listid/' . $this->getId());
        drupal_goto($destination['path']);
      }
      else {
        $query = array(
          'query' => array(
            'page' => $current_page - 1,
          ),
        );
        $destination = drupal_parse_url('islandora-bookmark/listid/' . $this->getId());
        drupal_goto($destination['path'], $query);
      }
    }
    // Export selected only.
    elseif ($form_state['clicked_button']['#name'] === 'export_selected') {
      $pids = array();
      $broke_pids = '';
      $broke_count = 0;
      foreach ($form_state['values']['bookmarks']['fieldset']['table'] as $checkbox => $value) {
        if ($value !== 0) {
          // Make sure we can actually access the object.
          if (islandora_object_load($value)) {
            $pids[] = $value;
          }
          else {
            $broke_pids .= $value . ' ';
            $broke_count++;
          }
        }
      }

      $key = $form_state['input']['islandora_bookmark_export_options_select'];

      if (count($pids) > 0 && $key !== 'default') {
        if (array_key_exists('islandora_bookmark_export_styles_' . $key, $form_state['input'])) {
          call_user_func($form_state['islandora_bookmark_export'][$key], $pids, $form_state['input']['islandora_bookmark_export_styles_' . $key]);
        }
        else {
          call_user_func($form_state['islandora_bookmark_export'][$key], $pids);
        }
      }
      elseif (count($pids) === 0 && $broke_count === 0) {
        drupal_set_message(t('Please select at least one @type.', array('@type' => variable_get('islandora_bookmark_type', 'bookmark'))), 'error');
      }
      elseif ($key === 'default') {
        drupal_set_message(t('Please select an export type.'), 'error');
      }
      else {
        if ($broke_count > 0) {
          $broken_pids = format_plural($broke_count, 'The following object could not be exported as it does not exist: !broke_pids.',
                         'The following objects could not be exported as they do no exist: !broke_pids.',
                         array('!broke_pids' => trim($broke_pids)));
          drupal_set_message($broken_pids, 'error');
        }
      }
    }
    elseif ($form_state['clicked_button']['#name'] === 'islandora_bookmark_remove_selected') {
      $pids = array();
      foreach ($form_state['values']['bookmarks']['fieldset']['table'] as $checkbox => $value) {
        if ($value !== 0) {
          $pids[] = $value;
        }
      }
      if (count($pids) > 0) {
        $this->removePids($pids);
        drupal_set_message(t('The selected @type have been removed from @listname.',
            array(
              '@listname' => $this->bookmarkName,
              '@type' => variable_get('islandora_bookmark_type', 'bookmark') . 's',
            )
          ));
        $current_pids = $form_state['islandora_bookmark_pids_on_page'];
        $current_page = $form_state['islandora_bookmark_current_page'];

        // Still pids left on the page.
        if ((count($pids) - $current_pids) != 0) {
          $query = array(
            'query' => array(
              'page' => $current_page,
            ),
          );
          $destination = drupal_parse_url('islandora-bookmark/listid/' . $this->getId(), array('query' => $query));
          drupal_goto($destination['path'], $query);
        }
        elseif ($current_page == 0) {
          $destination = drupal_parse_url('islandora-bookmark/listid/' . $this->getId());
          drupal_goto($destination['path']);
        }
        else {
          $query = array(
            'query' => array(
              'page' => $current_page - 1,
            ),
          );
          $destination = drupal_parse_url('islandora-bookmark/listid/' . $this->getId());
          drupal_goto($destination['path'], $query);
        }
      }
      else {
        drupal_set_message(t('Please select at least one @type.', array('@type' => variable_get('islandora_bookmark_type', 'bookmark'))), 'error');
      }
    }
    elseif ($form_state['clicked_button']['#name'] === 'islandora_bookmark_remove_all') {
      $remove_count = $this->getPidCount();
      $this->removeAllPids();

      drupal_set_message(t('All PIDs have been removed from the @remove_list list.',
          array(
            '@remove_list' => $this->bookmarkName,
          )));
    }
    // Export all pids.
    else {
      $pids = $this->getPids();
      $all_pids = array_filter($pids, 'islandora_object_load');
      $broke_pids = array_diff($pids, $all_pids);
      $broke_count = count($broke_pids);

      $key = $form_state['input']['islandora_bookmark_export_options_select'];
      if ($key !== 'default' && count($all_pids) > 0) {
        if (array_key_exists('islandora_bookmark_export_styles_' . $key, $form_state['input'])) {
          call_user_func($form_state['islandora_bookmark_export'][$key], $all_pids, $form_state['input']['islandora_bookmark_export_styles_' . $key]);
        }
        else {
          call_user_func($form_state['islandora_bookmark_export'][$key], $all_pids);
        }
      }
      elseif ($key === 'default') {
        drupal_set_message(t('Please select an export type.'), 'error');
      }
      else {
        if ($broke_count > 0) {
          $broken_pids = format_plural(
            $broke_count,
            'The following object was unable to be exported as it does not exist: @broke_pids.',
            'The following objects were unable to be exported as they do no exist: @broke_pids.',
            array(
              '@broke_pids' => implode(', ', $broke_pids),
            )
          );
          drupal_set_message($broken_pids, 'error');
        }
      }
    }
  }
}
