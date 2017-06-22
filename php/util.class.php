<?php

class util
{
  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  private final function __construct(){}

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  public static function initialize()
  {
    ini_set( 'display_errors', '1' );
    error_reporting( E_ALL | E_STRICT );
    ini_set( 'date.timezone', 'US/Eastern' );
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  public static function out( $msg )
  {
    printf( '%s: %s'."\n", date( 'Y-m-d H:i:s' ), $msg );
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  public static function error( $msg )
  {
    out( sprintf( 'ERROR! %s', $msg ) );
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  public static function uniquify_input( $input )
  {
    $output = array_unique( $input );
    if( false !== ( $key = array_search( 'na', $output ) ) )
    {
      unset( $output[$key] );
    }
    if( false !== ( $key = array_search( '', $output ) ) )
    {
      unset( $output[$key] );
    }
    return $output;
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  public static function flatten( $input, $delimiter = ', ' )
  {
    return is_array( $input ) ?
      ( 1 == count( $input ) ? current( $input ) : implode( $delimiter, $input ) ) : $input;
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  public static function permutations( array $array )
  {
    if( 1 == count( $array ) )
    {
      return $array[0];
    }

    $a = array_shift( $array );
    $b = permutations( $array );

    $return = array();
    foreach( $a as $value )
    {
      if( !empty( $value ) && !is_null( $value ) )
      {
        foreach( $b as $value2 )
        {
          $return[] = array_merge( array( $value ), (array) $value2 );
        }
      }
    }

    return $return;
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  public static function numeric_permutations( array $array )
  {
    if( 1 == count( $array ) )
    {
      return $array[0];
    }

    $a = array_shift( $array );
    $b = numeric_permutations( $array );

    $return = array();
    foreach( $a as $value )
    {
      if( is_numeric( $value ) )
      {
        foreach( $b as $value2 )
        {
          $return[] = array_merge( array( $value ), (array) $value2 );
        }
      }
    }

    return $return;
  }

/**
 * show a status bar in the console
 *
 * <code>
 * for($x=1;$x<=100;$x++){
 *
 *     show_status($x, 100);
 *
 *     usleep(100000);
 *
 * }
 * </code>
 *
 * @param   int     $done   how many items are completed
 * @param   int     $total  how many items are to be done total
 * @param   int     $size   optional size of the status bar
 * @return  void
 *
 */

  public static function show_status( $done, $total, $size = 30 )
  {
    static $start_time;

    // if we go over our bound, just ignore it
    if( $done > $total ) return;

    if( empty( $start_time ) ) $start_time = time();
    $now = time();

    $perc = (double)($done / $total);

    $bar = floor($perc*$size);

    $status_bar = "\r[";
    $status_bar .= str_repeat( '=', $bar );
    if($bar<$size)
    {
      $status_bar .= '>';
      $status_bar .= str_repeat( ' ', $size - $bar );
    }
    else
    {
      $status_bar.= '=';
    }

    $disp = number_format( $perc*100, 0 );

    $status_bar.= "] $disp % $done/$total";

    if( 0 < $done )
    {
      $rate = ($now - $start_time) / $done;
      $left = $total - $done;
      $eta = round( $rate * $left, 2 );

      $elapsed = $now - $start_time;

      $status_bar.=
        ' remaining: ' .
        gmdate( 'H:i:s', $eta ) .
        ' sec, elapsed: ' .
        gmdate( 'H:i:s', $elapsed ) .
        ' sec';
    }
    echo "$status_bar ";
    flush();

    // when done, send a newline
    if( $done == $total )
    {
      echo "\n";
    }
  }

  public static function startsWith( $haystack, $needle )
  {
    // search backwards starting from haystack length characters from the end
    return "" === $needle ||
           false !== strrpos($haystack, $needle, -strlen($haystack));
  }

  public static function endsWith( $haystack, $needle )
  {
    // search forward starting from end minus needle length characters
    return "" === $needle ||
           ( 0 <= ($temp = strlen($haystack) - strlen($needle)) &&
             false !== strpos($haystack, $needle, $temp) );
  }
}
