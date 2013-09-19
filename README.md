BUILD STATUS
------------
Current build status:
[![Build Status](https://travis-ci.org/Islandora/islandora_solution_pack_compound.png?branch=7.x)](https://travis-ci.org/Islandora/islandora_solution_pack_compound)

CI Server:
http://jenkins.discoverygarden.ca

CONTENTS OF THIS FILE
---------------------

 * summary
 * requirements
 * installation
 * configuration
 * customization


SUMMARY
-------

The Islandora Compound Object Solution Pack enables generic parent-child relationships between objects. The object view
of a compound object is replaced by the view of its first child object. The included "Islandora Compound Object
Navigation" block provides a thumbnail navigation of an object's siblings. A "Compound" management tab allows for the
addition and removal of parent and child objects for each object.

REQUIREMENTS
------------

Islandora 7.x is required to use the Islandora Compound Object Solution Pack.

INSTALLATION
------------

Enable the module and if the navigation controls are desired, enable and configure the "Islandora Compound Object
Navigation" block at /admin/structure/block.

CONFIGURATION
-------------

The module has documented configuration options at admin/islandora/compound_object.

CUSTOMIZATION
-------------

The "Islandora Compound Object Navigation" block can be themed. See theme_islandora_compound_prev_next().

DRUSH
-------------

A Drush command has been added, to be run from the command line (Terminal), that will update the existing rel-predicate of
existing compound objects to 'isConstituentOf'. It can be run with the drush command 'drush update_rels_predicate'. This command 
accpets no arguments.
