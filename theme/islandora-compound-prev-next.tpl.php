<?php

/**
 * @file
 * islandora-compound-object-prev-next.tpl.php
 *
 * @TODO: needs documentation about file and variables
 * $parent_label - Title of compound object
 * $child_count - Count of objects in compound object
 * $parent_url - URL to manage compound object
 * $previous_pid - PID of previous object in sequence or blank if on first
 * $next_pid - PID of next object in sequence or blank if on last
 * $siblings - array of PIDs of sibling objects in compound 
 * $folder_image_path - path to default folder image for missing/restricted thumbnails
 */
 
?>
 <div class="islandora-compound-prev-next">
 <span class="islandora-compound-title"><?php 
  print t('Part of: @parent (@count objects)', array('@parent' => $parent_label, '@count' => $child_count)); ?>
 <?php if ($parent_url): ?>
    <?php print l(t('manage parent'), $parent_url); ?>
 <?php endif; ?>
 </span><br/>

 <?php if (!empty($previous_pid)): ?>
   <?php print l(t('Previous'), 'islandora/object/' . $previous_pid); ?>
 <?php endif; ?>
 <?php if (!empty($previous_pid) && !empty($next_pid)): ?>
    | 
 <?php endif;?>
 <?php if (!empty($next_pid)): ?>
   <?php print l(t('Next'), 'islandora/object/' . $next_pid); ?>
 <?php endif; ?>

 <?php if ($child_count > 1): ?>
   <div class="islandora-compound-thumbs">
   <?php for ($i = 0; $i < count($siblings); $i++): ?>
     <div class="islandora-compound-thumb">
     <?php $sibling = $siblings[$i];
     if ($sibling === $pid) {
       $active = array('class' => 'active');
     } else {
       $active = array();
     }
     $sibling_object = islandora_object_load($sibling);
     if (isset($sibling_object['TN']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $sibling_object['TN'])) {
       $path = 'islandora/object/' . $sibling . '/datastream/TN/view';
     } else {
       // Object either does not have a thumbnail or it's restricted show a
       // default in its place.
       
       $path = $folder_image_path;
     }?>

     <span class='islandora-compound-caption'><?php print $sibling_object->label;?></span>

     <?php print l(
       theme_image(
         array(
           'path' => $path,
           'attributes' => array('class' => $active),
         )
       ),
       'islandora/object/' . $sibling,
       array('html' => TRUE)
     );?>

     </div>
   <?php endfor; // 0 -> count($siblings) ?>
   </div> <!-- /islandora-compound-thumbs -->
 <?php endif; // $child_count > 1 ?>
 </div>