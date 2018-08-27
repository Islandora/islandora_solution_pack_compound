<?php
/**
 * @file
 * Batch functionality for the compound solution pack.
 */

/**
 * Creates batch object for deleting the children of a compound.
 *
 * @param AbstractObject $compound
 *   The compound we are deleting from.
 * @param array $children
 *   An array of children to delete.
 *
 * @return array
 *   The batch array to be passed to batch_set.
 */
function islandora_compound_object_delete_children_batch(AbstractObject $compound, array $children) {
  $batch = array(
    'finished' => 'islandora_compound_object_batch_finished',
    'progress_message' => t('Time elapsed: @elapsed <br/>Estimated time remaining @estimate.'),
    'error_message' => t('An error has occurred.'),
    'file' => drupal_get_path('module', 'islandora_compound_object') . '/includes/batch.inc',
  );
  $message_parameters = array('@compound' => $compound->label);
  $child_count = count($children);
  $batch += array(
    'operations' => array(
      array(
        'islandora_compound_object_delete_children_batch_operation',
        array($compound, $children),
      ),
    ),
    'title' => format_plural($child_count,
      'Deleting 1 child from @compound ...',
      'Deleting @count children from @compound ...', $message_parameters),
    'init_message' => format_plural($child_count,
      'Preparing to delete 1 child from @compound ...',
      'Preparing to delete @count children from @compound ...', $message_parameters),
  );
  return $batch;
}

/**
 * Starts the batch operation.
 *
 * @param AbstractObject $compound
 *   The compound being deleted.
 * @param array $children
 *   The children of the compound being deleted.
 * @param array $context
 *   The batch context.
 *
 * @return mixed
 *   The child being processed in this invocation.
 */
function islandora_compound_object_start_operation(AbstractObject $compound, array $children, array &$context) {
  if (!isset($context['sandbox']['progress'])) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['total'] = count($children);
    $context['results'] = array(
      'deleted' => 0,
      'removed' => 0,
      'parent' => $compound->label,
    );
  }
  return $children[$context['sandbox']['progress']];
}

/**
 * Check if an object has other parents.
 *
 * Will return true if object has any relationship to anther object other then
 * its content model object.
 *
 * @param AbstractObject $child
 *   The child object.
 * @param AbstractObject $compound
 *   The parent compound.
 *
 * @return bool
 *   TRUE if the child has other parents, FALSE otherwise.
 */
function islandora_compound_object_has_other_parents(AbstractObject $child, AbstractObject $compound) {
  $relationships = $child->relationships->get();
  foreach ($relationships as $relationship) {
    $is_cmodel_relationship = $relationship['predicate']['namespace'] == FEDORA_MODEL_URI
      && $relationship['predicate']['value'] == 'hasModel';
    if (!$is_cmodel_relationship
      && $relationship['object']['literal'] == FALSE
      && $relationship['object']['value'] != $compound->id
    ) {
      return TRUE;
    }
  }
  return FALSE;
}

/**
 * Removes the compound relationships from the child.
 *
 * @param AbstractObject $child
 *   The child object.
 * @param AbstractObject $compound
 *   The parent object.
 */
function islandora_compound_remove_relationships(AbstractObject $child, AbstractObject $compound) {
  $rels_predicate = variable_get('islandora_compound_object_relationship', 'isConstituentOf');
  $escaped_pid = str_replace(':', '_', $compound->id);
  $child->relationships->autoCommit = FALSE;
  $child->relationships->remove(FEDORA_RELS_EXT_URI, $rels_predicate, $compound->id);
  $child->relationships->remove(ISLANDORA_RELS_EXT_URI, "isSequenceNumberOf$escaped_pid");
  $child->relationships->commitRelationships();
}

/**
 * Moves the progress of the batch operation. Ending if appropriate.
 *
 * @param array $context
 *   The batch context.
 */
function islandora_compound_object_end_operation(array &$context) {
  $context['sandbox']['progress']++;
  if ($context['sandbox']['progress'] < $context['sandbox']['total']) {
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['total'];
  }
  else {
    $context['finished'] = 1;
  }
}

/**
 * The batch operation to remove the children.
 *
 * Removes their relationships to the parent if they have other parents, and
 * deletes them otherwise.
 *
 * @param AbstractObject $compound
 *   The parent compound.
 * @param array $children
 *   The list of children being processed.
 * @param array $context
 *   The batch context.
 */
function islandora_compound_object_delete_children_batch_operation(AbstractObject $compound, array $children, array &$context) {
  $child = islandora_compound_object_start_operation($compound, $children, $context);
  if ($child && $child = islandora_object_load($child)) {
    if (islandora_compound_object_has_other_parents($child, $compound)) {
      // If more than one parent, only remove parent relationship.
      islandora_compound_remove_relationships($child, $compound);
      $context['results']['removed']++;
    }
    else {
      // If only one parent, delete the object.
      islandora_delete_object($child);
      $context['results']['deleted']++;
    }
  }
  islandora_compound_object_end_operation($context);
}

/**
 * Called when the batch is finished. Sets a success/fail message.
 *
 * @param bool $success
 *   Success of the batch.
 * @param array $results
 *   Results from the batch context.
 * @param array $operations
 *   Operations in the batch.
 */
function islandora_compound_object_batch_finished($success, $results, $operations) {
  if ($success) {
    $deleted = format_plural($results['deleted'],
      'Deleted 1 child from @compound.',
      'Deleted @count children from @compound.',
      array('@compound' => $results['parent'])
    );
    $removed = format_plural($results['removed'],
      'Removed 1 child from @compound.',
      'Removed @count children from @compound.',
      array('@compound' => $results['parent'])
    );
    if ($results['deleted']) {
      drupal_set_message($deleted);
    }
    if ($results['removed']) {
      drupal_set_message($removed);
    }
  }
  else {
    // Generic Error Message.
    $error_operation = reset($operations);
    $message = t('An error occurred while processing %error_operation with arguments: @arguments', array(
      '%error_operation' => $error_operation[0],
      '@arguments' => print_r($error_operation[1], TRUE)));
    drupal_set_message($message, 'error');
  }
}
