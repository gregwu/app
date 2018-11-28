<?php
class debug
{
	public static function trace()
	{
		debug::log(debug::callStack(debug_backtrace()));
	}


	public static function callStack($stacktrace) 
	{
		array_shift($stacktrace);
		$rstacktrace = array_reverse($stacktrace);
		foreach($rstacktrace as $node) 
		{
			$out .= basename($node['file']) ."(" .$node['line']."):";
			if(!empty( $node['class'] ))
			{
				$out .=  $node['class'].'->';
			}
			$out .= $node['function'] ." | ";
		}
		return $out;
        }

	public static function log($v, $where = '')
	{
		error_log("\n $where \n",3,"/tmp/debug.log");
		error_log(stripslashes(var_export($v, true))."\n\n" , 3, '/tmp/debug.log');
		error_log("\n\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n\n",3,"/tmp/debug.log");
	}

	public static function tracesql($sql, $where = '')
	{
		$out = debug::callStack(debug_backtrace());
		$sql = stripslashes(var_export($sql, true));
		$out .= substr($sql, 1, -1);
		error_log("\n $out \n",3,"/tmp/debug.log");
	}

} 

