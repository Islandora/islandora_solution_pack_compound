<?php

/**
 * @file
 * Holds derivative functionality for compound solution pack.
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
      );
    }
  }
}
