#!/usr/bin/php
<?php
/**
 *
 * @author Dean Inglis <inglisd@mcmaster.ca>
 */

require_once('../../../php/util.class.php');
require_once('../../../php/database.class.php');
util::initialize();

  function xml2assoc( &$xml )
  {
    $assoc = NULL;
    while( $xml->read() )
    {
      if( XMLReader::END_ELEMENT == $xml->nodeType )
        break;
      if( XMLReader::ELEMENT == $xml->nodeType && !$xml->isEmptyElement )
      {
        $key = $xml->name;
        if( $xml->hasAttributes )
        {
          while( $xml->moveToNextAttribute() )
            $assoc[$key][$xml->name] = $xml->value;
        }
        $assoc[$key] = xml2assoc( $xml );
      }
      else if( $xml->isEmptyElement )
      {
        break;
      }
      else if( XMLReader::TEXT == $xml->nodeType )
      {
        $assoc = $xml->value;
      }
    }

    return $assoc;
  }

  if( $argc < 2  )
  {
    util::error( 'usage: CleanUnratedCarotid.php path_to/config.xml <debug> <verbose>\n' );
    exit();
  }

  $filename = $argv[1];
  if( !file_exists( $filename ) || !is_readable( $filename ) )
    util::error( 'cannot open file ' . $filename );

  $xml = new XMLReader();
  $xml->open( $filename );

  $assoc = xml2assoc($xml);

  $dbkeys = array( 'Host','Port','Username','Password','Name' );
  $dbassoc = '';
  try
  {
    $dbassoc = $assoc['Configuration']['Database'];
    foreach( $dbkeys as $key )
      if( !array_key_exists( $key, $dbassoc) )
      {
        util::error( 'missing database configuration element; ', $key );
      }
  }
  catch( Exception $e )
  {
    util::error( $e->getMessage() );
    exit();
  }

  // open connection to the database
  $db = new database(
    $dbassoc['Host'],
    $dbassoc['Username'],
    $dbassoc['Password'],
    $dbassoc['Name'] );

  $pathkeys = array( 'Log','ImageData' );
  $pathassoc = '';
  try
  {
    $pathassoc = $assoc['Configuration']['Path'];
    foreach( $pathkeys as $key )
      if( !array_key_exists( $key, $pathassoc) )
      {
        util::error( 'missing path configuration element: ', $key );
      }
  }
  catch( Exception $e )
  {
    util::error( $e->getMessage() );
    exit();
  }
  $dataPath = realpath($pathassoc['ImageData']) . '/';

  $debug = 1;
  if( $argc > 2 )
    $debug = $argv[2];
  
  $verbose = 1;
  if( $argc > 3 )
    $verbose = $argv[3];

  $numProcessed = 0;

  $sql = 'SELECT CONCAT(CONCAT_WS("/",i.Id,e.Id,g.Id),".dcm") AS filename, '.
    'g.Id as imageid, '.
    'e.Id as examid '.
    'FROM Image g '.
    'JOIN Exam e ON e.Id=g.ExamId '.
    'JOIN Interview i ON i.Id=e.InterviewId '.
    'JOIN ScanType s ON s.Id=e.ScanTypeId '.
    'JOIN Wave w ON w.Id=i.WaveId '.
    'AND w.Id=s.WaveId '.
    'LEFT JOIN Rating r ON r.ImageId=g.Id '.
    'WHERE s.Type="CarotidIntima" '.
    'AND w.Rank=1 '.
    'AND g.Dimensionality=2 '.
    'AND r.Id IS NULL';

  $data = $db->get_all( $sql );

  if($verbose)
    util::out($sql);

  if($verbose)
    util::out( 'repairing ' . count( $data ) . ' baseline CarotidIntima image records having invalid or missing files' );
  $sql_str1 = 'UPDATE Image SET ParentImageId = NULL WHERE Id = %d';
  $sql_str2 = 'DELETE Image.* FROM Image WHERE Id = %d';

  if( 0 < count( $data ) )
  {
    foreach( $data as $elem )
    {
      $fileName = $dataPath . $elem['filename'];
      if(file_exists($fileName) && filesize($fileName)>0)
      {
        continue;
      }
      if($fileName != realpath($fileName))
      {
        if($verbose)
          util::out( $fileName . ' is invalid' );
        $sql = sprintf($sql_str1, $elem['imageid']);
        if($verbose)
          util::out($sql);
        if(0==$debug)
        {
          $db->execute($sql);
        }
        $sql = sprintf($sql_str2, $elem['imageid']);
        if($verbose)
          util::out($sql);
        if(0==$debug)
        {
          $db->execute($sql);
        }
        $numProcessed++;
      }
    }
  }
  util::out('number of image records deleted: ' . $numProcessed . ' of ' . count($data));
?>
