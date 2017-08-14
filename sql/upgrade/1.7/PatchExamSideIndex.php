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
    util::error( 'usage: PatchExamSideIndex.php path_to/config.xml <debug> <verbose>\n' );
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
  }

  $opalkeys = array( 'Host','Port','Username','Password','WaveSource' );
  $opalassoc = '';
  try
  {
    $opalassoc = $assoc['Configuration']['Opal'];
    foreach( $opalkeys as $key )
      if( !array_key_exists( $key, $opalassoc) )
      {
        util::error( 'missing opal configuration element; ', $key );
      }
  }
  catch( Exception $e )
  {
    util::error( $e->getMessage() );
  }

  // open connection to the database
  $db = new database(
    $dbassoc['Host'],
    $dbassoc['Username'],
    $dbassoc['Password'],
    $dbassoc['Name'] );

  $debug = 1;
  if( $argc > 2 )
    $debug = $argv[2];

  $verbose = 1;
  if( $argc > 3 )
    $verbose = $argv[3];

  $numExamProcessed = 0;
  $numOrphanedRatings = 0;

  // correct Exam records with SideIndex = null
  $sql = 'SELECT Exam.Id AS examid, '.
         'Interview.UId AS uid, '.
         'ScanType.Type AS type, '.
         'Wave.Name AS wave, '.
         'Wave.MetaDataSource AS source, '.
         'IFNULL(Image.Id,-1) AS imageid, '.
         'IFNULL(Rating.Id,-1) AS ratingid '.
         'FROM Exam '.
         'JOIN Interview ON Interview.Id=Exam.InterviewId '.
         'JOIN ScanType ON ScanType.Id=Exam.ScanTypeId '.
         'JOIN Wave ON Wave.Id=Interview.WaveId '.
         'AND Wave.Id=ScanType.WaveId '.
         'LEFT JOIN Image ON Image.ExamId=Exam.Id '.
         'LEFT JOIN Rating ON Rating.ImageId=Image.Id '.
         'WHERE Exam.SideIndex IS NULL '.
         'AND ScanType.SideCount > 1 '.
         'ORDER BY source, type, uid';

  $data = $db->get_all( $sql );

  if($verbose)
    util::out($sql);

  if($verbose)
    util::out( 'repairing ' . count( $data ) .' Exam records having null SideIndex' );
  $sql_str = 'UPDATE Exam SET SideIndex = %d WHERE Exam.Id = %d';

  if( 0 < count( $data ) )
  {
    $opalnet =
        ' -o https://' . $opalassoc['Host'] . ':' . $opalassoc['Port'] .
        ' -u ' . $opalassoc['Username'] .
        ' -p ' . $opalassoc['Password'];

    foreach( $data as $elem )
    {
      $opalvar = $elem['source'] . '.Exam:' . $elem['type'] . '.' . 'SideIndex ';
      $opalcmd = 'opal data ' . $opalvar . $opalnet . ' -i ' . $elem['uid'];
      if($verbose)
        util::out( $opalcmd );
      $res = json_decode(shell_exec($opalcmd), true);
      if(!empty($res) || !is_null($res) && array_key_exists('value',$res))
      {
        $index = $res['value'];
        $sql = sprintf($sql_str, $index, $elem['examid']);
        if($verbose)
          util::out($sql);
        if(0 == $debug)
        {
          $db->execute($sql);
          $numExamProcessed++;
        }
      }
      if(-1 == $elem['imageid'] && -1 != $elem['ratingid'])
      {
        if($verbose)
          util::out('found orphaned rating');
        $numOrphanedRatings++;
      }
    }

  }
  util::out('number of exam records updated: ' . $numExamProcessed . ' of ' . count($data));
  util::out('number of orphaned ratings: ' . $numOrphanedRatings . ' of ' . count($data));
?>
