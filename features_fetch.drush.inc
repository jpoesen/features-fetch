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
    drush_log(dt('Retrieved config file'), 'ok');
    
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
     drush_die(dt('Failed to retrieve !incfiledir/features_fetch.drush.ini : File not found.'), 
     array('!incfiledir' => dirname(__FILE__)));
  }
  
  if ($files) {
    // sort file list by key (filedate, newest first)
    krsort($files);

    // figure out where drush is installed and load backup.inc
    // which we need to backup our Feature module before replacing it
    // TODO: figure out a better way to include this file
    $list = drush_commandfile_list();
    $updatecode_include_path = str_replace('pm.drush.inc', 'version_control/backup.inc', $list['pm']);
    require_once $updatecode_include_path;

    $drupal_root = drush_get_context('DRUSH_DRUPAL_ROOT');
    $project['tarname'] = array_shift($files);
    $project['name'] = basename($project['tarname'], ".tar");
    $project['path'] = $config['feature_destination_path'] . $project['name'];
    $project['full_project_path'] = $drupal_root . '/' . $project['path'];

    $msg = dt('Retrieved !tarname', array('!tarname' => $project['tarname']));
    drush_log($msg, 'ok');

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
      $msg = dt('Copied !tarname to !dest', 
        array(
          '!tarname' => $project['tarname'], 
          '!dest' => $config['feature_destination_path'],
        )
      );
      drush_log($msg, 'ok');
      
      if ($config['cleanup'] == 1){
        if (file_exists($config['download_path'] . $project['tarname'])) {
          if(unlink($config['download_path'] . $project['tarname'])){
            $msg = dt('Cleaned up !tarname', array('!tarname' =>  $project['tarname']));
            drush_log($msg, 'ok');
          } else {
            $msg = dt('Unable to remove !tarname from !downpath', 
              array(
                '!tarname' => $project['tarname'], 
                '!downpath' => $config['download_path'],
              )
            );
            drush_set_error($msg);
          }
        }
      }
    } else {
      $msg = dt('Failed to copy !src to !dest',
        array(
          '!src' => $config['download_path'] . $project['tarname'],
          '!dest' => $config['feature_destination_path'] . '/' . $featurename . '/',
        )
      );
      drush_set_error($msg);
    }    
  } else {
    $msg = dt('Failed to retrieve a <!featurename> Feature at !downpath', 
      array(
      '!featurename' => $featurename,
      '!downpath' => $config['download_path'],
      )
    ); 
    drush_set_error($msg);
  }
  
  if (drush_get_error_log()){
    $msg = dt('OH NOES! Drush could not complete fetching Feature <!featurename>', 
      array('!featurename' => $featurename));
    drush_set_error($msg);
  } else {
    $msg = dt('SUCCESS! Feature <!featurename> was fetched.', 
      array('!featurename' => $featurename));
    drush_log($msg, 'ok');
  }
}


