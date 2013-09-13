<?php

/**
 * @file
 * Islandora Compound Object Drush
 * Drush statements to update a compound objects relations with children
 * from 'isPartOf' to 'isConstituentOf'.
 *
 * Hook implementations and general functions.
 */

/**
 * Implements hook_drush_help().
 */
function islandora_compound_object_drush_help($section) {
  switch ($section) {
    case 'drush:update_rels_predicate':
      return dt("update relationship predicate on compound objects from isPartOf to isConstituentOf.");
  }
}

/**
 * Set up the drush command.
 *
 * @return array
 *   The drush command in an array.
 */
function islandora_compound_object_drush_command() {
  $items = array();
  $items['update_rels_predicate'] = array(
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
    'callback' => 'islandora_compound_object_setup_batch',
    'description' => dt('Update relationship predicate on compound object children from isPartOf to isConstituentOf'),
  );
  return $items;
}

/**
 * Set up the batch process to update the rels_predicate.
 */
function islandora_compound_object_setup_batch() {
  // Update all compound object child relationships to use
  // isConstituentOf.
  module_load_include('inc', 'islandora', 'includes/utilities');

  $tuque = islandora_get_tuque_connection();
  $query = 'select $object $label from <#ri> where (
            $object <fedora-model:label> $label and
            $object <info:fedora/fedora-system:def/model#hasModel> <info:fedora/islandora:compoundCModel> and
            $object <fedora-model:state> <info:fedora/fedora-system:def/model#Active>)
            order by $label';
  $results = $tuque->repository->ri->itqlQuery($query, 'unlimited');
  $operations = array();
  foreach ($results as $compound_object) {
    array_push($operations, array("islandora_compound_object_batch_set_rels_predicate",
      array($compound_object['object']['value'], 'Setting child relationship(s) predicate from compound object: ',
      ),
      )
    );
    // Now set up the operation to set the new predicate. In this case,
    // fedora:isConstituentOf
    array_push($operations, array("islandora_compound_object_batch_delete_rels_predicate",
      array($compound_object['object']['value'], 'Deleting old child relationship(s) predicate from compound object: ',
      ),
      )
    );
  }

  // Construct the batch array for processing.
  $batch = array(
    'operations' => $operations,
    'title' => t('Rels predicate update batch'),
    'finished' => 'islandora_compound_object_batch_update_rels_predicate_finished',
    'file' => drupal_get_path('module', 'islandora_compound_object') . '/includes/islandora_compound_object.drush.inc',
  );

  // Get the batch process set up.
  batch_set($batch);
  $batch =& batch_get();
  $batch['progressive'] = FALSE;
  drush_backend_batch_process();
}

/**
 * Set the rels_predicate of the compound object member.
 *
 * @param array $compound_object
 *   The PID of the compound object.
 * @param array $operation_details
 *   The status message to for this context.
 * @param array $context
 *   The current processing context.
 */
function islandora_compound_object_batch_set_rels_predicate($compound_object, $operation_details, &$context) {
  $context['message'] = $operation_details . $compound_object;
  $parts = islandora_compound_object_get_parts($compound_object);
  $insert_seq = islandora_compound_object_get_insert_sequence($parts);
  foreach ($parts as $part) {
    $escaped_pid = str_replace(':', '_', $compound_object);
    $child_object = islandora_object_load($part);
    $child_object->relationships->add('info:fedora/fedora-system:def/relations-external#', 'isConstituentOf', $compound_object);
    $child_object->relationships->add(ISLANDORA_RELS_EXT_URI, "isSequenceNumberOf$escaped_pid", $insert_seq, TRUE);
  }
}

/**
 * Remove the rels_predicate of the compound object member.
 *
 * @param string $compound_object
 *   The AbstractFedoraObject pid.
 * @param array $operation_details
 *   Details of current operation being processed.
 * @param array $context
 *   Active context of the current batch.
 */
function islandora_compound_object_batch_delete_rels_predicate($compound_object, $operation_details, &$context) {
  $context['message'] = $operation_details . $compound_object;
  $parts = islandora_compound_object_get_parts($compound_object);
  foreach ($parts as $part) {
    $escaped_pid = str_replace(':', '_', $compound_object);
    $child_object = islandora_object_load($part);
    $child_object->relationships->remove('info:fedora/fedora-system:def/relations-external#', 'isPartOf', $compound_object);
    $child_object->relationships->remove(ISLANDORA_RELS_EXT_URI, "isSequenceNumberOf$escaped_pid");
  }
}

/**
 * Retrieve the new insertion point.
 *
 * @param array $children
 *   The array of the compound object's children.
 *
 * @return int
 *   The position to insert the new compound object.
 */
function islandora_compound_object_get_insert_sequence($children) {
  $insert_seq = 0;
  foreach ($children as $child) {
    if (!empty($child['seq']) && $child['seq'] > $insert_seq) {
      $insert_seq = $child['seq'];
    }
  }
  // Want to insert one past this point.
  $insert_seq++;
  return $insert_seq;
}

/**
 * Batch process complete handler.
 *
 * @param array $success
 *   The batch status success message.
 * @param array $results
 *   The batch staus result.
 * @param array $operations
 *   The batch operations performed.
 */
function islandora_compound_object_batch_update_rels_predicate_finished($success, $results, $operations) {
  // Print finished message to user.
  variable_set('islandora_compound_object_relationship', 'isConstituentOf');
  drush_print('Finished updating compound object relationship predicate(s).');
}
