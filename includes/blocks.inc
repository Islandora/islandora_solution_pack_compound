<?php

/**
 * @file
 * Holds block definitions for the compound solution pack.
 */

/**
 * Constructs the JAIL display block for a compound.
 *
 * @return array
 *   Renderable array for the block.
 */
function islandora_compound_object_jail_display_block() {
  $block = array();
  $jail_path = libraries_get_path('JAIL');
  $module_path = drupal_get_path('module', 'islandora_compound_object');
  drupal_add_css("$module_path/css/islandora_compound_object.jail_block.css");
  drupal_add_js("$jail_path/lib/jquery.js", array('group' => JS_LIBRARY));
  drupal_add_js("$jail_path/dist/jail.min.js", array('group' => JS_LIBRARY));
  drupal_add_js("$module_path/js/compound_jail.js", array('group' => JS_LIBRARY));
  $object = menu_get_object('islandora_object', 2);
  if ($object) {
    // If only compound cmodel objects can be compound, this isn't necessary.
    $children = islandora_compound_object_get_parts($object);
    $pid = (!empty($children)) ? $children[0] : $object->id;
    $compound_object = islandora_object_load($pid);
    $compound_info = islandora_compound_object_retrieve_compound_info($compound_object);
    $output = '';
    if ($compound_info) {
       // This object is part of a compound object.
      $output .= theme('islandora_compound_jail_prev_next', $compound_info);
      if (!empty($output)) {
        return array('#markup' => $output);
      }
    }
  }
}

/**
 * Constructs the default navigation block for a compound.
 *
 * @param AbstractObject $object
 *   If supplied the object to create the block for.
 *
 * @return array
 *   Renderable array for the block.
 */
function islandora_compound_object_navigation_block(AbstractObject $object = NULL) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  drupal_add_css(drupal_get_path('module', 'islandora_compound_object') . '/css/islandora_compound_object.block.css');
  $object = $object ? $object : menu_get_object('islandora_object', 2);
  if ($object) {
    // If only compound cmodel objects can be compound, this isn't necessary.
    $children = islandora_compound_object_get_parts($object->id);
    $pid = (!empty($children)) ? $children[0] : $object->id;
    $object = islandora_object_load($pid);
    $output = '';
    if ($object) {
      $compound_info = islandora_compound_object_retrieve_compound_info($object);
      // This object is part of a compound object.
      if (!empty($compound_info)) {
        $prev_next = theme('islandora_compound_prev_next', $compound_info);
        $output .= $prev_next;
      }

      if (!empty($output)) {
        return array('#markup' => $output);
      }
    }
  }
}
