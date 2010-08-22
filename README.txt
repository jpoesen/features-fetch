DESCRIPTION
-----------
Features Fetch (ff)
is a simple drush command extension that simplifies deploying (rebuilt) Drupal 
Feature modules to a specific Drupal project. The rationale for this module is 
explained in the 'what it actually does' section below.

This extension reuses core drush functionality (backing up, unpacking, etc) as 
much as possible.

Drush
is a command line shell and scripting interface for Drupal
(http://drupal.org/project/drush)

Features
is a Drupal module enables the capture and management of features in Drupal. 
A feature is a collection of Drupal entities which taken together 
satisfy a certain use-case.
(http://drupal.org/project/features)


INSTALLATION
------------
  1. If you haven't already done so, create a .drush directory under your home directory.

    $ mkdir ~/.drush # if necessary

  2. Clone this repo to $HOME/.drush/

    $ cd ~/.drush
    $ git clone git://github.com/jpoesen/features-fetch.git


  3. open up $HOME/.drush/features_fetch/features_fetch.drush.ini and specify your
download path, features destination path and whether or not to clean up afterwards.

download path: 
  The path where your browser commonly saves downloads to
  example: /Users/alice/Downloads/
  
features destination path: 
  The path where you commonly store your custom features modules
  example: sites/all/modules/custom/features/
  note: these settings are machine-wide. Project-specific settings will be added soon. 

cleanup: 
  default value = 1. 
  set to 0 to remove the downloaded feature module .tar file from your downloads directory 
  after successful deployment.

note: drush command extensions can be installed at alternative locations. Consult 
/path/to/drush/README.txt for more details.


USAGE
-----
From within a Drupal project, run:
  $ drush ff <example_feature_module>
  

WHAT IT ACTUALLY DOES
---------------------
1. Backs up any existing version of <example_feature_module> to DRUPALROOT/backups
2. Grab the most recent <example_feature_module>.tar file from your downloads directory
3. unpacks it to your features destination directory

This means that after every feature module rebuild you can simply download it via the Features 
admin interface and then deploy it with a single drush ff command.


TODO
----
* provide project-specific download path and features destination path
* code cleanup

