<?php

/**
 * @file
 * includes/manage.form.inc
 *
 * Compound object management callback and form.
 */

/**
 * Menu callback for manage form.
 */
function islandora_compound_object_manage(AbstractObject $object) {
  return drupal_get_form('islandora_compound_object_manage_form', $object);
}

/**
 * Compound management form.
 */
function islandora_compound_object_manage_form($form, &$form_state, $object) {
  $form = array();
  $rels_predicate = variable_get('islandora_compound_object_relationship', 'isPartOf');

  // Add child objects
  if (variable_get('islandora_compound_object_compound_children', TRUE) && in_array(ISLANDORA_COMPOUND_OBJECT_CMODEL, $object->models)) {
    $form['add_children'] = array(
      '#type' => 'fieldset',
      '#title' => t('Add Child Objects'),
      '#description' => t('Add child objects as part of this compound object'),
    );
    $form['add_children']['child'] = array(
      '#type' => 'textfield',
      '#title' => t('Child Object Pid/Label'),
      '#autocomplete_path' => 'islandora_compound_object/autocomplete',
    );

    // Remove children
    $children = islandora_compound_object_get_parts($object->id, TRUE);
    if (!empty($children)) {
      $form['children'] = array(
        '#type' => 'fieldset',
        '#title' => t('Remove Child Objects'),
        '#description' => t('Remove child objects of as part of this compound object'),
      );

      $header = array('title' => t('Title'), 'pid' => t('Object ID'));
      $form['children']['remove_children'] = array(
        '#type' => 'tableselect',
        '#title' => t('Children'),
        '#header' => $header,
        '#options' => $children,
      );
    }
  }

  // Add parents
  $form['add_to_parent'] = array(
    '#type' => 'fieldset',
    '#title' => t('Associate with Parent Object'),
    '#description' => t('Add this object to a parent object'),
  );
  $form['add_to_parent']['parent'] = array(
    '#type' => 'textfield',
    '#title' => t('Parent Object Pid/Label'),
    '#autocomplete_path' => 'islandora_compound_object/autocomplete',
  );

  // Remove parents
  $parent_part_of = $object->relationships->get('info:fedora/fedora-system:def/relations-external#', $rels_predicate);
  if (!empty($parent_part_of)) {
    $form['parents'] = array(
      '#type' => 'fieldset',
      '#title' => t('Unlink From Parent'),
      '#description' => t('Remove the relationship between this object and parent objects.'),
    );

    $parents = array();
    foreach ($parent_part_of AS $parent) {
      // Shouldn't be too much of a hit but would be good to avoid the object_loads
      $pid = $parent['object']['value'];
      $parent_object = islandora_object_load($pid);
      $parents[$pid] = array('title' => $parent_object->label, 'pid' => $pid);
    }

    $form['parents']['unlink_parents'] = array(
      '#type' => 'tableselect',
      '#title' => t('Parents'),
      '#header' => array('title' => t('Title'), 'pid' => t('Object ID')),
      '#options' => $parents
    );
  }

  $form['object'] = array(
    '#type' => 'value',
    '#value' => $object,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );

  return $form;
}

/**
 * Compound object management validation.
 */
function islandora_compound_object_manage_form_validate($form, &$form_state) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  $object = $form_state['values']['object'];
  $rels_predicate = variable_get('islandora_compound_object_relationship', 'isPartOf');

  // Child
  if (!empty($form_state['values']['child'])) {
    // Check if valid child pid
    if (!islandora_is_valid_pid($form_state['values']['child'])) {
      form_set_error('child', t('Invalid object supplied.'));
    }

    // Do not allow child of self
    if ($form_state['values']['child'] == $object->id) {
      form_set_error('child', t('An object may not be a child of itself.'));
    }

    // Do not allow repeated child
    $child_object = islandora_object_load($form_state['values']['child']);
    $child_part_of = $child_object->relationships->get('info:fedora/fedora-system:def/relations-external#', $rels_predicate);
    foreach ($child_part_of AS $part) {
      if ($part['object']['value'] == $object->id) {
        form_set_error('child', t('The object is already a parent of the child.'));
      }
    }
  }

  // Parent
  if (!empty($form_state['values']['parent'])) {
    // Check if valid parent pid
    if (!islandora_is_valid_pid($form_state['values']['parent'])) {
      form_set_error('parent', t('Invalid object supplied.'));
    }

    // Do not allow parent of self
    if ($form_state['values']['parent'] == $object->id) {
      form_set_error('parent', t('An object may not be the parent of itself.'));
    }

    // Do not allow repeated parent
    $parent_part_of = $object->relationships->get('info:fedora/fedora-system:def/relations-external#', $rels_predicate);
    foreach ($parent_part_of AS $part) {
      if ($part['object']['value'] == $form_state['values']['parent']) {
        form_set_error('parent', t('The object is already a child of the parent.'));
      }
    }
  }

  if (!empty($form_state['values']['parent']) && !empty($form_state['values']['child']) && $form_state['values']['parent'] == $form_state['values']['child']) {
    form_set_error('child', t('An object may not be the parent and child of the same object.'));
    form_set_error('parent');
  }

}

/**
 * Compound object management submit.
 */
function islandora_compound_object_manage_form_submit($form, &$form_state) {
  $object = $form_state['values']['object'];
  $create_thumbs = variable_get('islandora_compound_object_thumbnail_child', TRUE);
  $rels_predicate = variable_get('islandora_compound_object_relationship', 'isPartOf');

  // Relationship from child to this object
  if (!empty($form_state['values']['child'])) {
    $child_object = islandora_object_load($form_state['values']['child']);
    $child_object->relationships->add('info:fedora/fedora-system:def/relations-external#', $rels_predicate, $object->id);

    if ($create_thumbs) {
      islandora_compound_object_update_parent_thumbnail($child_object, $object);
    }
  }

  // Add relationship to parent
  if (!empty($form_state['values']['parent'])) {
    $object->relationships->add('info:fedora/fedora-system:def/relations-external#', $rels_predicate, $form_state['values']['parent']);

    if ($create_thumbs) {
      islandora_compound_object_update_parent_thumbnail($object, islandora_object_load($form_state['values']['parent']));
    }
  }

  // Remove children
  // @TODO: batch
  if (!empty($form_state['values']['remove_children'])) {
    foreach ($form_state['values']['remove_children'] AS $child) {
      if ($child) {
        $child_object = islandora_object_load($child);
        $child_object->relationships->remove('info:fedora/fedora-system:def/relations-external#', $rels_predicate, $object->id);
      }
    }
  }

  // Unlink parents
  if (!empty($form_state['values']['unlink_parents'])) {
    foreach ($form_state['values']['unlink_parents'] AS $parent) {
      if ($parent) {
        $object->relationships->remove('info:fedora/fedora-system:def/relations-external#', $rels_predicate, $parent);
      }
    }
  }

  // @TODO: actually confirm
  drupal_set_message(t('Compound relationships modified.'));
}
