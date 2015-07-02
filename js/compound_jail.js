/**
 * @file
 * Add JAIL for fancy loading of images.
 */

(function ($) {
    Drupal.behaviors.islandora_compound_object_JAIL = {
        attach: function(context, settings) {
            $('img.islandora-compound-object-jail').jail({
                triggerElement:'#block-islandora-compound-object-compound-jail-display .islandora-compound-jail-prev-next',
                event: 'scroll'
            });
        }
    };
})(jQuery.noConflict(true));
