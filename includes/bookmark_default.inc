<?php

/**
 * @file
 * Definition of a Bookmark object.
 */

/**
 * Class that defines a BookmarkDefaultList object. A BookmarkDefautList object
 * only difference is that it cannot be deleted.
 */
class BookmarkDefaultList extends BookmarkDatabase {
  /**
   * Constructor for the BookmarkDefaultList object.
   */
  public function __construct($list_id) {
    parent::__construct($list_id);
    $this->isDeletable = FALSE;
  }

  /**
   * Overwrite to do nothing...
   */
  public function delete() {

  }
}
