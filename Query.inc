<?php
include 'debug.inc';


class Query
{
   var $param; 

   // Class Constructor
   function Query()
   {
	session_start();





	$descriptorspec = array(
	   0 => array("pipe", "r"),  // stdin is a pipe that the child will read from
	   1 => array("pipe", "w")  // stdout is a pipe that the child will write to
	);
/*	
	// if the GET parameter "generateKeypair" is set
	if(isset($_GET["getPublicKey"])) {
		$arrOutput = array(
			"publickey" => file_get_contents('rsa_1024_pub.pem')
		);
		// Convert the response to JSON, and send it to the client
		echo json_encode($arrOutput);
		// else if the GET parameter "decrypttest" is set
	} elseif (isset($_GET["handshake"])) {
		// Decrypt the client's request
		$cmd = sprintf("openssl rsautl -decrypt -inkey rsa_1024_priv.pem");
		$process = proc_open($cmd, $descriptorspec, $pipes);
		if (is_resource($process)) {
		    fwrite($pipes[0], base64_decode($_POST['key']));
		    fclose($pipes[0]);

		    $key = stream_get_contents($pipes[1]);
		    fclose($pipes[1]);
		    proc_close($process);
		}
		// Save the AES key into the session
		$_SESSION["key"] = $key;
		
		// JSON encode the challenge
		$cmd = sprintf("openssl enc -aes-256-cbc -pass pass:'$key' -a -e");
		$process = proc_open($cmd, $descriptorspec, $pipes);
		if (is_resource($process)) {
		    fwrite($pipes[0], $key);
		    fclose($pipes[0]);

		    // we have to trim all newlines and whitespaces by ourself
		    $challenge = trim(str_replace("\n", "", stream_get_contents($pipes[1])));
		    fclose($pipes[1]);
		    proc_close($process);
		}

		echo json_encode(array("challenge" =>  $challenge));
		// echo json_encode(array("challenge" => AesCtr::encrypt($key, $key, 256)));
	} elseif (isset($_GET["decrypttest"])) {
		// set timezone just in case
		date_default_timezone_set('UTC');
		// Get some test data to encrypt, this is an ISO 8601 timestamp
		$toEncrypt = date("c");

		// get the key from the session
		$key = $_SESSION["key"];

		$cmd = sprintf("openssl enc -aes-256-cbc -pass pass:'$key' -a -e");
		$process = proc_open($cmd, $descriptorspec, $pipes);
		if (is_resource($process)) {
		    fwrite($pipes[0], $toEncrypt);
		    fclose($pipes[0]);

		    $encrypted = stream_get_contents($pipes[1]);
		    fclose($pipes[1]);
		    proc_close($process);
		}

		echo json_encode( 
			array(
				"encrypted" => $encrypted,
				"unencrypted" => $toEncrypt
			)
		);
	// else if the GET parameter "handshake" is set
	} else */ if (isset($_POST['jCryption'])) {
		$key = $_SESSION["key"];

		// Decrypt the client's request and send it to the clients(uncrypted)
		$cmd = sprintf("openssl enc -aes-256-cbc -pass pass:'$key' -d");
		$process = proc_open($cmd, $descriptorspec, $pipes);
		if (is_resource($process)) {
		    fwrite($pipes[0], base64_decode($_POST['jCryption']));
		    fclose($pipes[0]);

		    $data = stream_get_contents($pipes[1]);
		    fclose($pipes[1]);
		    proc_close($process);
		}
		parse_str($data, $this->param);
//debug::log($this->param);
/*		
		if(strtolower($_SERVER['HTTP_X_REQUESTED_WITH']) == 'xmlhttprequest') {
			echo json_encode(array("data" => $data));
		} else {
			parse_str($data, $output);
			print_r($output);
		}
*/




	} 
	else
	{

		$this->param  = array();
		$this->setParams();
	}
	//debug($this->param, 'Query');
   }

	function echoo($name, $d = '')
	{
		echo $this->get($name, $d);
	}

	function checked($name, $value)
	{
		echo $this->get($name) == $value? 'checked' : '';
	}

	function selected($name, $value)
        {
                echo $this->get($name) == $value? 'selected' : '';
        }


	function get($name, $default = '')
	{
		if(!isset($this->param[$name]))
		{
			$this->param[$name] = $default;
		}
		return $this->param[$name];
	}

	function set($name, $value, $overwrite = true)
	{
		if(!isset($this->param[$name]) || $overwrite)
		{
			$this->param[$name] = $value;
		}
	}


    function has($name)
    {
        return isset($this->param[$name]) ? true : false;
    }

    function isEmpty($name)
    {
        return isset($this->param[$name]) && !empty($this->param[$name])? false : true;
    }

   // Class Methods

   // read form variables
   // read GET variables, but let POST variables override them
   function setParams()
   {
      foreach( $_GET as $k => $v ) 
      {
            if(is_string($v))
            {
                $this->param[$k] = $v;
            }
            else if(is_array($v))
            {
                $this->param[$k] = $v;
                //$this->param[$k.'[]'] = $v;
            }
            else
            {
                $this->param[$k] = NULL;
            }

         //$this->param[$k] = is_string($v) ? strip_tags($v) : $v;
         //$this->param[$k] = $this->param[$k] == '' ? NULL : $this->param[$k];
      }

      foreach( $_POST as $k => $v ) 
      {
            if(is_string($v))
            {
                $this->param[$k] = $v;
            }
            else if(is_array($v))
            {
                $this->param[$k] = $v;
                //$this->param[$k.'[]'] = $v;
            }
            else
            {
                $this->param[$k] = NULL;
            }
//         $this->param[$k] = is_string($v) ? strip_tags($v) : $v;
 //        $this->param[$k] = $this->param[$k] == '' ? NULL : $this->param[$k];
      }
   }




   function clearParams()
   {
      foreach( $_GET  as $k => $v ) unset($_GET[$k]);
      foreach( $_POST as $k => $v ) unset($_POST[$k]);
   }   
}

