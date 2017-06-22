<?php

require_once('util.class.php');

class database
{
  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  public function __construct( $server, $username, $password, $database, $charset = 'utf8' )
  {
    $this->server = $server;
    $this->username = $username;
    $this->password = $password;
    $this->name = $database;
    $this->connection = new \mysqli( $this->server, $this->username, $this->password, $this->name );
    if( $this->connection->connect_error )
    {
      util::error( 'Unable to connect to database, quiting' );
      die();
    }
    $this->connection->set_charset( $charset );
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  public function get_name() { return $this->name; }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  public function execute( $sql )
  {
    $result = $this->connection->query( $sql );
    if( false === $result )
    {   
      util::out( $this->connection->error );
      util::out( $sql );      
      return false;
    }
    return true;
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  public function get_all( $sql )
  {
    $result = $this->connection->query( $sql );
    if( false === $result )
    {   
      util::out( $this->connection->error );
      util::out( $sql );      
      return false;
    }
    $rows = array();
    while( $row = $result->fetch_assoc() ) $rows[] = $row;
    $result->free();
    return $rows;
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  public function get_row( $sql )
  {
    $result = $this->connection->query( $sql );
    if( false === $result )
    {   
      util::out( $this->connection->error );
      util::out( $sql );      
      return false;
    }
    $row = $result->fetch_assoc();
    $result->free();
    return $row;
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  public function get_one( $sql )
  {
    $result = $this->connection->query( $sql );
    if( false === $result )
    {   
      util::out( $this->connection->error );
      util::out( $sql );      
      return false;
    }
    $array = $result->fetch_array( MYSQLI_NUM );
    $result->free();
    $value = is_null( $array ) ? NULL : current( $array );
    return $value;
    
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  public function insert_id()
  {
    $id = $this->connection->insert_id;
    return $id;
  }

  protected $connection; 
  private $server;
  private $username;
  private $password;
  private $name;
}
