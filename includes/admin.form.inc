<?php

/**
 * @file
 * includes/admin.form.inc
 *
 * Module admin form.
 */

/**
 * Module admin form.
 */
function islandora_compound_object_admin_form($form, &$form_state) {
  $form = array();

  $form['islandora_compound_object_compound_children'] = array(
    '#type' => 'checkbox',
    '#title' => t('Only allow compound objects to have child objects associated with them'),
    '#description' => t('If unchecked, all objects may have child objects.'),
    '#default_value' => variable_get('islandora_compound_object_compound_children', TRUE),
  );

  $form['islandora_compound_object_thumbnail_child'] = array(
    '#type' => 'checkbox',
    '#title' => t('Generate a thumbnail for compound objects from their first child'),
    '#description' => t('If checked, the thumbnail for a compound object will be generated from its first child object.'),
    '#default_value' => variable_get('islandora_compound_object_thumbnail_child', TRUE),
  );

  $form['islandora_compound_object_relationship'] = array(
    '#type' => 'textfield',
    '#title' => t('Child relationship predicate'),
    '#description' => t('Changing this after objects have been created will break functionality. Should be part of info:fedora/fedora-system:def/relations-external#'),
    '#default_value' => variable_get('islandora_compound_object_relationship', 'isConstituentOf'),
    '#required' => TRUE,
  );

  $form['islandora_compound_object_solr_fq'] = array(
    '#type' => 'textfield',
    '#title' => t('Solr filter query'),
    '#description' => t('Filter compound object children from Solr results.'),
    '#default_value' => variable_get('islandora_compound_object_solr_fq', '-RELS_EXT_isConstituentOf_uri_mt:*'),
  );

  return system_settings_form($form);
}
