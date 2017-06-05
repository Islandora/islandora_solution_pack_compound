<?php

/**
 * @file
 * Holds block definitions for the compound solution pack.
 */

/**
 * Derivative creation function for compound parent object TN.
 *
 * Wraps the TN update/creation function and
 * adds expected logging.
 *
 * @param AbstractObject $object
 *   The compound object.
 * @param bool $force
 *   Boolean for forcing this derivative creation.
 *
 * @return array
 *   Success message.
 */
function islandora_compound_object_create_thumbnail(AbstractObject $object, $force = FALSE) {
  $parts = islandora_compound_object_get_parts($object->id);
  if (!empty($parts)) {
    $child = islandora_object_load($parts[0]);
    islandora_compound_object_update_parent_thumbnail($object, $child);
  }
  if (isset($child) && $object['TN'] == $child['TN']) {
    return array(
      'success' => TRUE,
      'messages' => array(
      array(
        'message' => t('Updated @dsid on @parent.'),
        'message_sub' => array(
          '@dsid' => 'TN',
          '@parent' => $object->id,
        ),
        'type' => 'dsm',
        'severity' => 'status',
      ),
      ),
      'file' => 'islandora_compound_object.module',
    );
  }
  else {
    return array(
      'success' => FALSE,
      'messages' => array(
      array(
        'message' => t('Failed to update @dsid on @parent.'),
        'message_sub' => array(
          '@dsid' => 'TN',
          '@parent' => $object->id,
        ),
        'type' => 'watchdog',
        'severity' => WATCHDOG_ERROR,
      ),
      ),
      'file' => 'islandora_compound_object.module',
    );
  }
}

/**
 * Update parent from child object.
 *
 * Given a child object, check to see if it is part of
 * a compound and update the parent's TN datastream.
 *
 * @param AbstractObject $child
 *   The potential child object.
 * @param bool $force
 *   Boolean for forcing this derivative creation.
 *
 * @return array|NULL
 *   Success message.
 */
function islandora_compound_object_create_thumbnail_on_parent(AbstractObject $child, $force = FALSE) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  // Test if child has a parent.
  $compound_info = islandora_compound_object_retrieve_compound_info($child);
  if (empty($compound_info)) {
    return NULL;
  }
  // Test if child is the first child of the compound.
  if (empty($compound_info['previous_pid'])) {
    $parent = islandora_object_load($compound_info['parent_pid']);
    // Child doesn't have a thumbnail then we can't set the parents TN either.
    if (empty($child['TN'])) {
      // Parents with first children with no TN shouldn't have one either.
      if (isset($parent['TN'])) {
        $parent->purgeDatastream('TN');
      }
      return NULL;
    }
    islandora_compound_object_update_parent_thumbnail($parent, $child);
    if (isset($parent['TN']) && $parent['TN'] == $child['TN']) {
      return array(
        'success' => TRUE,
        'messages' => array(
        array(
          'message' => t('Updated @dsid on @parent.'),
          'message_sub' => array(
            '@dsid' => 'TN',
            '@parent' => $parent->id,
          ),
          'type' => 'dsm',
          'severity' => 'status',
        ),
        ),
        'file' => 'islandora_compound_object.module',
      );
    }
    else {
      return array(
        'success' => FALSE,
        'messages' => array(
        array(
          'message' => t('Failed to update @dsid on @parent.'),
          'message_sub' => array(
            '@dsid' => 'TN',
            '@parent' => $parent->id,
          ),
          'type' => 'watchdog',
          'severity' => WATCHDOG_ERROR,
        ),
        ),
        'file' => 'islandora_compound_object.module',
      );
    }
  }
  else {
    return array(
      'success' => FALSE,
      'messages' => array(
      array(
        'message' => t('Failed to update @dsid of @child.'),
        'message_sub' => array(
          '@dsid' => 'TN',
          '@child' => $child->id,
        ),
        'type' => 'watchdog',
        'severity' => WATCHDOG_ERROR,
      ),
      ),
      'file' => 'islandora_compound_object.module',
    );
  }
}

/**
 * Update a parent object's TN by deriving it's first child's TN.
 *
 * @param AbstractObject $parent
 *   The compound object.
 * @param AbstractObject $child
 *   The child object (optional).
 */
function islandora_compound_object_update_parent_thumbnail(AbstractObject $parent, AbstractObject $child = NULL) {
  // Short circuit if feature is disabled.
  if (!variable_get('islandora_compound_object_tn_deriv_hooks', FALSE)) {
    return;
  }
  module_load_include('inc', 'islandora', 'includes/utilities');

  $parts = islandora_compound_object_get_parts($parent->id);
  if (!empty($parts)) {
    if ($child == NULL) {
      $child = islandora_object_load($parts[0]);
    }
    // Child doesn't have a thumbnail then we can't set the parents TN either.
    if (empty($child['TN'])) {
      return;
    }
    $mime_detector = new MimeDetect();
    $ext = $mime_detector->getExtension($child['TN']->mimeType);
    // Windows will likely store temp data in a temp directory
    // rather than in memory. Since the colon is forbidden in
    // filenames, replace it with an underscore instead.
    $thumbnail_id = ((islandora_deployed_on_windows()) ? str_replace(':', '_', $child->id) : $child->id);
    $file = drupal_realpath("temporary://{$thumbnail_id}_TN.{$ext}");
    $child['TN']->getContent($file);
    if (empty($parent['TN'])) {
      $ds = $parent->constructDatastream('TN', 'M');
      $ds->setContentFromFile($file, TRUE);
      $ds->label = $child['TN']->label;
      $ds->mimetype = $child['TN']->mimetype;
      $parent->ingestDatastream($ds);
    }
    else {
      if ($parent['TN']->label != $child['TN']->label) {
        $parent['TN']->label = $child['TN']->label;
      }
      if ($parent['TN']->mimeType != $child['TN']->mimetype) {
        $parent['TN']->mimetype = $child['TN']->mimetype;
      }
      $parent['TN']->setContentFromFile($file, TRUE);
    }
    file_unmanaged_delete($file);
  }
  elseif (isset($parent['TN'])) {
    $parent->purgeDatastream('TN');
  }
}