<?php

/**
 * @file
 * Holds backend callable definitions for the compound solution pack.
 */

/**
 * Implements callback_islandora_compound_object_query_backends().
 */
function islandora_compound_object_legacy_query_sparql($pid) {
  $rels_predicate = variable_get('islandora_compound_object_relationship', 'isConstituentOf');
  $objects = array();

  $connection = islandora_get_tuque_connection();
  $escaped_pid = str_replace(':', '_', $pid);
  if ($connection) {
    $query = <<<EOQ
PREFIX islandora-rels-ext: <http://islandora.ca/ontology/relsext#>
SELECT ?object ?title ?seq
FROM <#ri>
WHERE {
  ?object <fedora-model:label> ?title ;
          <fedora-rels-ext:$rels_predicate> <info:fedora/$pid> .
  OPTIONAL {
    ?object islandora-rels-ext:isSequenceNumberOf$escaped_pid ?seq
  }
}
EOQ;
    $results = $connection->repository->ri->sparqlQuery($query);

    // Sort the objects into their proper order.
    $sort = function($a, $b) {
      $a = $a['seq']['value'];
      $b = $b['seq']['value'];
      if ($a === $b) {
        return 0;
      }
      if (empty($a)) {
        return 1;
      }
      if (empty($b)) {
        return -1;
      }
      return $a - $b;
    };
    uasort($results, $sort);

    foreach ($results as $result) {
      $object = islandora_object_load($result['object']['value']);
      if (islandora_object_access(ISLANDORA_VIEW_OBJECTS, $object)) {
        $objects[$result['object']['value']] = array(
          'pid' => $result['object']['value'],
          'title' => $result['title']['value'],
          'seq' => $result['seq']['value'],
        );
      }
    }
  }

  return $objects;
}

/**
 * Implements callback_islandora_compound_object_query_backends().
 */
function islandora_compound_object_query_sparql($pid) {
  return islandora_compound_object_query_sparql_general($pid, FALSE);
}

/**
 * Implements callback_islandora_compound_object_query_backends().
 */
function islandora_compound_object_query_sparql_compliant($pid) {
  return islandora_compound_object_query_sparql_general($pid, TRUE);
}

/**
 * General callback function for SPARQL query functions above.
 */
function islandora_compound_object_query_sparql_general($pid, $cast_int = FALSE) {
  module_load_include('inc', 'islandora', 'includes/utilities');

  $rels_predicate = variable_get('islandora_compound_object_relationship', 'isConstituentOf');
  $objects = array();

  // Handle XACML restrictions in query.
  global $user;
  if ($user->uid === 0) {
    $user_name = 'anonymous';
  }
  else {
    $user_name = $user->name;
  }

  $role_matches = array();
  foreach ($user->roles as $role) {
    $role_matches[] = "?role='{$role}'";
  }
  $role_matcher = implode(' || ', $role_matches);

  // Handle namespace restrictions in query.
  $enforced = variable_get('islandora_namespace_restriction_enforced', FALSE);
  $namespace_filter = '';
  if ($enforced) {
    $namespace_array = islandora_get_allowed_namespaces();
    $namespace_sparql = implode('|', $namespace_array);
    $namespace_filter = format_string('FILTER(regex(str(?object), "info:fedora/(!namespaces):"))', array('!namespaces' => $namespace_sparql));
  }

  $escaped_pid = str_replace(':', '_', $pid);

  // XXX: The default Mulgara triple store sorts the sequence number as an
  // integer, even though it doesn't have the integer type. Other SPARQL
  // compliant triple stores (specifically blazegraph) may do a string sort
  // on this, leading to the results being sorted strangely. Mulgara doesn't
  // support casting datatypes, but any SPARQL compatible store should. This
  // lets compatible stores return the results in the int sort order.
  if ($cast_int) {
    $sort = 'ORDER BY ASC(xsd:integer(?seq))';
  }
  else {
    $sort = 'ORDER BY ASC(?seq)';
  }

  $query = <<<EOQ
PREFIX islandora-rels-ext: <http://islandora.ca/ontology/relsext#>
SELECT DISTINCT ?object ?title ?seq
FROM <#ri>
WHERE {
  ?object <fedora-model:label> ?title ;
          <fedora-rels-ext:$rels_predicate> <info:fedora/$pid> .
  OPTIONAL {
      ?object islandora-rels-ext:isSequenceNumberOf$escaped_pid ?seq
  }
  OPTIONAL {{
      ?object islandora-rels-ext:isViewableByRole ?role
    } UNION {
      ?object islandora-rels-ext:isViewableByUser ?user
  }}
  FILTER((!bound(?role) && !bound(?user)) || (bound(?user) && ?user='{$user_name}') || (bound(?role) && ($role_matcher)))
  $namespace_filter
}
$sort
EOQ;

  $connection = islandora_get_tuque_connection();
  $results = $connection ? $connection->repository->ri->sparqlQuery($query) : array();

  foreach ($results as $result) {
    $objects[$result['object']['value']] = array(
      'pid' => $result['object']['value'],
      'title' => $result['title']['value'],
      'seq' => $result['seq']['value'],
    );
  }

  return $objects;
}
