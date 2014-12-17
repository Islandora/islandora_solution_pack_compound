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
 * $single_child_url - link to access object, if there is only one child
 * $themed_siblings - array of siblings of model
 *    array(
 *      'pid' => PID of sibling,
 *      'label' => label of sibling,
 *      'TN' => URL of thumbnail or default folder if no datastream,
 *      'class' => array of classes for this sibling,
 *    )
 */
 
?>
 <div class="islandora-compound-prev-next">
 <span class="islandora-compound-title"><?php 
  print t('Part of: @parent (@count objects)', array('@parent' => $parent_label, '@count' => $child_count)); ?>
 <?php if ($parent_url && $parent_pid != $pid): ?>
    <?php print l(t('manage parent'), $parent_url); ?>
 <?php endif; ?>
 <br/>
 <?php if ($single_child_url && $parent_pid == $pid): ?>
    <?php print l(t('Link to only child object'), $single_child_url); ?><br />
 <?php endif; ?>
 </span>
fix
 <?php if (!empty($previous_pid)): ?>
   <?php print l(t('Previous'), 'islandora/object/' . $previous_pid); ?>
 <?php endif; ?>
 <?php if (!empty($previous_pid) && !empty($next_pid)): ?>
    | 
 <?php endif;?>
 <?php if (!empty($next_pid)): ?>
   <?php print l(t('Next'), 'islandora/object/' . $next_pid); ?>
 <?php endif; ?>

 <?php if (count($themed_siblings) > 0): ?>
   <div class="islandora-compound-thumbs">
   <?php foreach ($themed_siblings as $sibling): ?>
     <div class="islandora-compound-thumb">
     <span class='islandora-compound-caption'><?php print $sibling['label'];?></span>
     <?php print l(
       theme_image(
         array(
           'path' => $sibling['TN'],
           'attributes' => array('class' => $sibling['class']),
         )
       ),
       'islandora/object/' . $sibling['pid'],
       array('html' => TRUE)
     );?>
     </div>
   <?php endforeach; // each themed_siblings ?>
   </div> <!-- // islandora-compound-thumbs -->
 <?php endif; // count($themed_siblings) > 0 ?>
 </div>