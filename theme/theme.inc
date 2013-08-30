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
function theme_islandora_compound_prev_next($variables) {
  drupal_add_js(drupal_get_path('module', 'islandora_compound_object') . '/js/caption_width.js');

  $output = '<div class="islandora-compound-prev-next">';
  $output .= '<span class="islandora-compound-title">' . t('Part of: @parent (@count objects)',
    array('@parent' => $variables['parent_label'], '@count' => $variables['child_count']));

  if ($variables['parent_url']) {
    $output .= ' ' . l(t('manage parent'), $variables['parent_url']);
  }
  $output .= '</span><br/>';

  if (!empty($variables['previous_pid'])) {
    $output .= l(t('Previous'), 'islandora/object/' . $variables['previous_pid']);
  }
  if (!empty($variables['previous_pid']) && !empty($variables['next_pid'])) {
    $output .= ' | ';
  }
  if (!empty($variables['next_pid'])) {
    $output .= l(t('Next'), 'islandora/object/' . $variables['next_pid']);
  }

  if ($variables['child_count'] > 1) {
    $output .= '<div class="islandora-compound-thumbs">';
    for ($i = 0; $i < count($variables['siblings']); $i++) {
      $output .= '<div class="islandora-compound-thumb">';
      $sibling = $variables['siblings'][$i];
      if ($sibling === $variables['pid']) {
        $active = array('class' => 'active');
      }
      else {
        $active = array();
      }
      $sibling_object = islandora_object_load($sibling);
      if (isset($sibling_object['TN']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $sibling_object['TN'])) {
        $path = 'islandora/object/' . $sibling . '/datastream/TN/view';
      }
      else {
        // Object either does not have a thumbnail or it's restricted show a
        // default in its place.
        $image_path = drupal_get_path('module', 'islandora');
        $path = "$image_path/images/folder.png";
      }

      $output .= "<span class='islandora-compound-caption'>{$sibling_object->label}</span>";

      $output .= l(
        theme_image(
          array(
            'path' => $path,
            'attributes' => array('class' => $active),
          )
        ),
        'islandora/object/' . $sibling,
        array('html' => TRUE)
      );

      $output .= '</div>';
    }
    $output .= '</div>';
  }

  $output .= '</div>';

  return $output;
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
