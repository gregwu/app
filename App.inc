<?php
require_once("Query.inc");
require_once("dbconnection.php");
require_once("Template.class.inc");
//require_once("mc.inc");
date_default_timezone_set('America/Los_Angeles');

class App extends Query
{
	function App()
	{
		parent::Query();
		$this->basename = basename($_SERVER['SCRIPT_FILENAME'],'.php');
		$isjson = $this->basename[0] == '_';
		$this->user_id = isset($_SESSION['username'])? $_SESSION['username'] : '' ;
		$this->domain =  isset($_SESSION['domain'])? $_SESSION['domain'] : '';
		$this->db = DBConnection::instantiate();
		$this->t = new Template('./html/');


		if($this->auth())
		{
			if($isjson)
			{
				header('Content-type: application/json');
				echo $this->run();
			}
			else
			{
				$this->run();
				echo $this->t->setch($this, $this->basename.".html"); 
			}
		}
		else
		{
			if($isjson)
			{
				header('Content-type: application/json');
                                echo '{"status":"error","msg":"please login"}';
			}
			else
			{
				header("location:login.php?msg=Please Login&url=".$this->basename.'.php');
			}
		}
	}

	function auth()
	{
		if(!isset($_SESSION['username']) )
		{
			$this->msg = 'Please login';
			return false;
		}

		if($this->has('node_id'))
		{
			$this->node_id = $this->get('node_id');
			
			$this->node = $this->verify($this->node_id);
			if($this->node )
			{
				return true;
			}
		}

		$this->msg = 'access denied'; 
		return false;
	}

	function verify($node_id)
	{
		$node = $this->db->findone('node', array('_id'=>new MongoDB\BSON\ObjectID($node_id)));
//debug::log($node);
//debug::log($node->updated_by.' == '.$this->user_id);
		if($node->updated_by == $this->user_id)
		{
			return $node;
		}
		return false;
	}

        function run()
        {

        }
}

