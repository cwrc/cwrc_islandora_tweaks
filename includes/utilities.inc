<?php

/**
 * Searches through available collection objects,removes results if the user doesn't have access.
 *
 * @param string $search_value
 *   Returns a json array of matching collections.
 */
function cwrc_islandora_tweaks_get_collections_filtered($search_value) {
  global $user;
  module_load_include('inc', 'islandora_basic_collection', 'includes/utilities');
  $tuque = islandora_get_tuque_connection($user);
  $sparql_query = <<<EOQ
SELECT ?pid ?label
WHERE {
  ?pid <fedora-model:label> ?label ;
       <fedora-model:hasModel> <info:fedora/islandora:collectionCModel> .
  FILTER(regex(?label, '$search_value', 'i') || regex(str(?pid), '$search_value', 'i'))
}
EOQ;
  $results = $tuque->repository->ri->sparqlQuery($sparql_query);
  $return = array();
  $pids = array();
  foreach ($results as $objects) {

    $object = islandora_object_load($objects['pid']['value']);
    if (isset($object)) {
      $parents = islandora_basic_collection_get_parent_pids($object);
      $parent_labels = array();
      foreach ($parents as $parent) {
        $parent_object = islandora_object_load($parent);
        $parent_labels[] = $parent_object->label;
      }
      $access = islandora_xacml_editor_islandora_object_access('administer islandora_xacml_editor',$object, $user);
  
      $return[$objects['pid']['value']] = t('@label [@parents] (@pid)', array(
        '@label' => $objects['label']['value'],
        '@parents' =>  implode(', ', $parent_labels),
        '@pid' => $objects['pid']['value'],
      ));
  
      $object = islandora_object_load($objects['pid']['value']);
      $access = islandora_xacml_editor_islandora_object_access('administer islandora_xacml_editor',$object, $user);
  
      if(empty($access)) {
        unset($return[$objects['pid']['value']]);
      }
    }
  }
  drupal_json_output($return);
}
