<?php

/**
 * Implementation of hook_drush_command().
 */
function features_fetch_drush_command() {
  $items['features-fetch'] = array(
    'description' => dt('Fetches a Feature with a newer version.'),
    'callback' => 'features_fetch_execute',
    'arguments' => array(
      'featurename' => dt('Required. Module name (machine name) of the Feature to fetch.'),     
    ),    
    //'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_DATABASE,
    'aliases' => array('ff'),
  );
  return $items;
}

/**
 * Implementation of hook_drush_help().
 */
function features_fetch_drush_help($section) {
  switch ($section) {
    case 'drush:features-fetch':
      return dt("drush features fetch <featurename>");
  }
}

/**
 * Drush callback
 */
function features_fetch_execute($featurename) {
  // grab some config settings
  if ($config = @parse_ini_file(dirname(__FILE__) . '/features_fetch.drush.ini')){
    // log success
    drush_log('Retrieved config file', 'ok');
    
    // scan download directory for one or more versions of our Feature module
    $dirhandle = opendir($config['download_path']);
    $files = array();
    while (false !== ($filename = readdir($dirhandle))) {
      // we're only interested in tar files that start with the name of our feature
      if (preg_match("(^$featurename(.*)\.tar$)", $filename)){ 
        $date = filemtime($config['download_path'] . $filename);
        $files[$date] = $filename; 
      }
    } 
    closedir($dirhandle);
  } else {
     drush_die('Failed to retrieve' . dirname(__FILE__) . '/features_fetch.drush.ini : File not found.');
  }
  
  if ($files) {
    // sort file list by key (filedate, newest first)
    krsort($files);

    // figure out where drush is installed and load backup.inc
    // which we need to backup our Feature module before replacing it
    $list = drush_commandfile_list();
    $updatecode_include_path = str_replace('pm.drush.inc', 'version_control/backup.inc', $list['pm']);
    require_once $updatecode_include_path;

    $drupal_root = drush_get_context('DRUSH_DRUPAL_ROOT');
    $project['tarname'] = array_shift($files);
    $project['name'] = basename($project['tarname'], ".tar");
    $project['path'] = $config['feature_destination_path'] . $project['name'];
    $project['full_project_path'] = $drupal_root . '/' . $project['path'];

    drush_log('Retrieved ' . $project['tarname'], 'ok');

    // only perform a backup if the Feature module was already installed
    $backup = new drush_pm_version_control_backup;
    if (is_dir($config['feature_destination_path'] . $project['name'])){
      $backup->pre_update($project);
    }

    // fetch our new feature module and put it where the backed up one used to be
    // command to build: tar -xf "path/to/downloads/example.tar" -C path/to/custom/features/directory
    $tar_cmd = 'tar -xf ';
    $tar_cmd .= '"' . $config['download_path'] . $project['tarname'] . '" ';
    $tar_cmd .= " -C " . $drupal_root . "/" . $config['feature_destination_path'];

    if (drush_shell_exec($tar_cmd)){
      drush_log('Copied ' . $project['tarname'] . ' to ' . $config['feature_destination_path'], 'ok');
    } else {
      drush_set_error('Failed to copy ' . $config['download_path'] . $project['tarname'] . '.tar to ' . $config['feature_destination_path'] . '/' . $featurename . '/');
    }    
  } else {
    drush_set_error('Failed to retrieve a <' . $featurename . '> Feature at '. $config['download_path']); 
  }
  
  if (drush_get_error_log()){
    drush_set_error('OH NOES! Drush could not complete fetching Feature <' . $featurename . '>');
  } else {
    drush_log('SUCCESS! Feature <' . $featurename . '> was fetched.', 'ok');
  }
}


