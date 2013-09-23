<?php

/**
 * @file
 * theme/theme.inc
 *
 * Theme functions.
 */

/**
 * Theme function for the compound navigation block parts.
 */
function islandora_compound_object_preprocess_islandora_compound_object_prev_next(&$variables) {
  drupal_add_js(drupal_get_path('module', 'islandora_compound_object') . '/js/caption_width.js');

  $image_path = drupal_get_path('module', 'islandora');
  $variables['folder_image_path'] = "$image_path/images/folder.png";


}

/**
 * Implements theme_element().
 */
function theme_islandora_compound_object_draggable_table(array $variables) {
  $form = $variables['form'];
  $rows = array();
  $weight_class = 'islandora-page-weight';
  $table = &$form['table'];
  foreach (element_children($table['rows']) as $id) {
    $row = &$table['rows'][$id];
    $row['weight']['#attributes'] = array('class' => array($weight_class));
    $rows[] = array(
      'data' => array(
        drupal_render($row['label']),
        drupal_render($row['pid']),
        drupal_render($row['weight']),
      ),
      'class' => array('draggable'),
    );
  }
  $table_id = 'islandora-reorder-objects-table';
  $output = theme('table', array(
              'header' => $table['#header'],
              'rows' => $rows,
              'attributes' => array('id' => $table_id)));
  $output .= drupal_render_children($form);
  drupal_add_tabledrag($table_id, 'order', 'sibling', $weight_class);
  return $output;
}
