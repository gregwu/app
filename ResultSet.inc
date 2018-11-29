<?php

ini_set('memory_limit','-1');

$CONSTS = get_defined_constants();
if (!isset($CONSTS['APPEND']))    define ( 'APPEND', FALSE );
if (!isset($CONSTS['NO_APPEND'])) define ( 'NO_APPEND', TRUE );

class ResultSet extends Queue
{
	// Initialize class variables
	var $result_set    = array();
	var $param_set    = array();
	var $_Current_Item = 0;
	var $_Last_Item    = 0;
	var $_Sql          = '';
	var $insert_id     = null;
	var $_Fields       = array();
	var $_DB_Link;
	var $_DB_Host = null;
	var $_DB_Port = 3306;
	var $_DB_Name;
	var $_DB_User;
	var $_DB_Pswd;
	var $db_file;
	var $is_die = true;



	function __construct($input_db)
	{
		$this->is_die = true;
		if ( file_exists( $input_db ) )
		{
			$this->db_file =  $input_db ;
		}
		else
		{
			$error = "please provide a valid database info file!\n";
			if($this->is_die)
			{ 		
				die($error);
			}
			else
			{  
				throw new Exception($error);
			}
		}

		require_once $this->db_file;
		$class_name = preg_match("/\/([\w_-]+)\.inc/", $this->db_file, $matches);
		$DB = new $matches[1];
		if(isset($DB->hostname))
		{
			list($this->_DB_Host, $this->_DB_Port) = explode(":", $DB->hostname)+ Array(1 => 3306);
		}

		$this->_DB_Name  = isset($DB->database)? $DB->database : '';   
		$this->_DB_User  = isset($DB->username)? $DB->username : '';   
		$this->_DB_Pswd  = isset($DB->password)? $DB->password : '';   

		// Connect to Database
		$this->connect_DB();

		// Define destructor method
		register_shutdown_function(array(&$this, "_DESTRUCTOR"));
	}


	// Connect to a database
	function connect_DB()
	{
//error_log("mysql:host={$this->_DB_Host};port={$this->_DB_Port};dbname={$this->_DB_Name};user=$this->_DB_User;pwd=$this->_DB_Pswd"."\n\n\n",3,"/tmp/sql.log");
		try 
		{
			if($this->_DB_Host != null)
			{  
				$this->_DB_Link = new PDO("mysql:host={$this->_DB_Host};port={$this->_DB_Port};dbname={$this->_DB_Name}", $this->_DB_User, $this->_DB_Pswd, array( PDO::ATTR_PERSISTENT => false));
			}


		}
		catch ( PDOException $e) 
		{
			if($this->is_die)
			{
				 die($e->getMessage()) ;
			}
			else
			{  
				throw new Exception($e->getMessage());
			}
		}
	}


	// Get table field attributes (name, type, attributes) 
	function tableFields($table_name)
	{
		$sql = "SHOW COLUMNS FROM $table_name";
		//error_log($sql."\n\n\n",3,"/tmp/sql.log");
		$this->_Fields       = array();
		
		$statement = $this->_DB_Link->query($sql);
		if(!$statement)
		{
			$error = "Table $table_name does not exist.";
			if($this->is_die)
			{
				die($error);
			}
			else
			{
				throw new Exception($error);
			}		
		}

		foreach($this->_DB_Link->query($sql) as $row)
		{
			$this->_Fields[$row['Field']]['type']  = $row['Type'];
			$this->_Fields[$row['Field']]['flags'] = $row['Extra'];
		}
	}

	// Insert or Update a table record
	// $key: Primary Key; DEFAULT='id'
	function Save($table_name, $key='id')
	{
		$this->tableFields($table_name);
		$has_prepare = false;
		if(!isset($this->update_set))return;
//error_log(var_export($this->update_set, true)."\n\n\n",3,"/tmp/sql.log");
		foreach ( $this->update_set as $row )
		{
			$this->param_set = array();
			if ( isset($row[$key]) )
			{
				// Record Update
				$pair  = Null;
				$pairs = array();
				$sql   = "UPDATE $table_name SET ";

				foreach ( $row as $prop => $value )
				{
					// Add only valid table fields which exist in the set  and are writeable
					if ( isset($this->_Fields[$prop]) ) 
					{
						if ( !strstr ( $this->_Fields[$prop]['flags'], 'auto_increment') ) 
						{
							if(!$has_prepare)
							{
								$pair = $prop . "= :" . $prop . ' ';
								array_push($pairs, $pair); 
							}
						}
						$this->param_set[':'.$prop] = $value;
					}
				}
				
				

//error_log(var_export($this->param_set, true)."\n\n\n",3,"/tmp/sql.log");
				if(!$has_prepare)
				{
					// Convert array to comma delimited string; add to query; exec
					$sql   .= implode(',', $pairs) . "WHERE $key = :$key";
					//$sql   .= implode(',', $pairs) . "WHERE $key = '". $row->$key."'";
//error_log($sql."\n\n\n",3,"/tmp/sql.log");
					$updateStatement = $this->_DB_Link->prepare($sql);
					$has_prepare = true;
				}
				$updateStatement->execute($this->param_set);


			} 
			else 
			{
				// Record Insertion
				$fields = array();
				$values = array();
				$sql    = "INSERT INTO $table_name (";

				foreach ( $row as $prop => $value )
				{  
					// Add only valid table fields which exist in the set  and are writeable
					if ( isset($this->_Fields[$prop]) ) 
					{
						if(!$has_prepare)
						{
							array_push($fields, $prop);
							array_push($values, ":".$prop);
						}
						$this->param_set[":".$prop] = $value;
					}
				}
//error_log($obj_prop .'=>'. $prop_value."\n",3, '/tmp/sql.log' );

				if(!$has_prepare)
				{
					// Convert arrays to comma delimited strings; add to query; exec
					$field_str = implode(',', $fields);
					$value_str = implode(',', $values);
					$sql      .= $field_str . ") values (" . $value_str . ")";
//error_log($sql."\n\n\n",3,"/tmp/sql.log");
					$insertStatement = $this->_DB_Link->prepare($sql);
					$has_prepare = true;
				}
				$insertStatement->execute($this->param_set);
//error_log(var_export($this->param_set, true)."\n\n\n",3,"/tmp/sql.log");

				$this->insert_id = $this->_DB_Link->lastInsertId(); 
			}

		}

		$this->update_set = array();
		return count($this->param_set);
	}


	// Get set (queue) object from a mySQL table query result set
	// $mode: APPEND|NO_APPEND DEFAULT=NO_APPEND
	function make_set($sql, $mode=NO_APPEND)
	{
//error_log(var_export($this->param_set, true)."\n\n\n",3,"/tmp/sql.log");
//error_log($sql."\n\n\n",3,"/tmp/sql.log");
		if ($mode) $this->clear();

		$sth = $this->_DB_Link->prepare($sql);
		try
		{
			$sth->execute($this->param_set);
		}
		catch ( Exception $e)
		{
			if($this->is_die)
                        {
                                 die($e->getMessage()."\n\n".$sql) ;
                        }
                        else
                        {
                                throw $e;
                        }
	
		}

		$results = $sth->fetchAll (PDO::FETCH_OBJ);      
		if (! empty($results) ) 
		{
			foreach($results AS $row) 
			{
				// Enqueue the row
				$this->enqueue($row);
			}
		}    

		return count($this->result_set);
	}


        function upsert($key = 'id')
        {
                $this->tableFields($this->table);
                $has_prepare = false;
                if(!isset($this->update_set))return;
		$values = array();
                foreach ( $this->update_set as $row )
                {
                        $fields = array();
                        $updates = array();
                        $param_set = array();

                        foreach ( $row as $prop => $value )
                        {
                                if ( isset($this->_Fields[$prop]) )
                                {
                                        if(!$has_prepare)
                                        {
                                                array_push($fields, $prop);
						array_push($updates, " $prop=VALUES($prop) ");
                                        }
                                        $param_set[] = $this->_DB_Link->quote($value);
                                }
                        }

                        $values[] = '('.implode(',',$param_set).')';

                        if(!$has_prepare)
                        {
                                $field_str = implode(',', $fields);
                                $update_str = implode(',', $updates);
                                $has_prepare = true;
                        }
                }

		if(count($values) == 0) return;

                $value_str = implode(",", $values);
                $sql = "INSERT INTO $this->table ( $field_str ) values  $value_str  ON DUPLICATE KEY UPDATE $update_str";
//error_log($sql."\n\n\n",3,"/tmp/sql.log");
                return $this->exec($sql);

        }



        function exec($sql)
        {

//error_log(var_export($this->param_set, true)."\n\n\n",3,"/tmp/sql.log");
//error_log($sql."\n\n\n",3,"/tmp/sql.log");
            try
            {
                return  $this->_DB_Link->exec($sql);
            }
            catch ( PDOException $e)
            {
                if($this->is_die)
		{
			 die($e->getMessage()."\n".var_export($this->param_set, true)."\n\n".$sql) ;
		}
		else
		{
			throw $e;
		}
            }
        }


	function execute_query($sql)
	{       

//error_log(var_export($this->param_set, true)."\n\n\n",3,"/tmp/sql.log");
//error_log($sql."\n\n\n",3,"/tmp/sql.log");
		$sth = $this->_DB_Link->prepare($sql);
		try
		{
			$sth->execute($this->param_set);
		}
		catch ( PDOException $e)
		{
			if($this->is_die)
                        {
                                 die($e->getMessage()."\n".var_export($this->param_set, true)."\n\n".$sql) ;
                        }
                        else
                        {
                                throw $e;
                        }
		}

	}


	function count()
	{
		return count($this->result_set);
	}




	// Class destructor
	function _DESTRUCTOR()
	{
		unset($this->result_set);
		if($this->_DB_Host!= null)
		{
		      //mysqli_close($this->_DB_Link);
			$this->_DB_Link =  null;   
		}

	}
}


// Define the queue class
class Queue
{
   // Initialize class variables
   var $result_set;
   var $_Current_Item = 0;
   var $_Last_Item    = 0;


   // Class Constructor
   function __construct()
   {
      $result_set     = array();

      // Define destructor method
      register_shutdown_function(array(&$this, "_DESTRUCTOR"));
   }


   // This function adds an item to the end of the queue
   function enqueue($object)
   {
	if(isset($object->id))
	{
		$this->result_set[$object->id] = $object;
	}
	else
	{
	      // Increment the last item counter
	      $this->_Last_Item = count($this->result_set);

	      // Add the item to the end of the queue
	      $this->result_set[$this->_Last_Item] = $object;
	}
   }


   // This function removes an item from the front of the queue
   function dequeue()
   {
      // If the queue is not empty...
      if(! $this->is_empty())
      {
         // Get the object at the front of the queue
         $object = $this->result_set[$this->_Current_Item];

         // Remove the object at the front of the queue
         unset($this->result_set[$this->_Current_Item]);

         // Increment the current item counter
         $this->_Current_Item++;

         // Return the object
         return $object;
      } else {
         // If the queue is empty...
         // Return a null value
         return null;
      }
   }


   // This function specifies whether or not the queue is empty
   function is_empty()
   {
      // If the queue is empty...
      if($this->_Current_Item > $this->_Last_Item)

         // Return a value of true
         return true;

      // If the queue is not empty...
      else

         // Return a value of false
         return false;
   }


   // This function empties the queue
   function clear()
   {
        $this->result_set = array();
   }


   // This function sorts the queue by a property
   // $sort_type: 0 = ascending; 1 = descending
   // $flag:      PHP Compare method
   function sort_by_property($property,$sort_type=0,$flag=SORT_REGULAR)
   {
      if (isset($this->result_set) and count($this->result_set) > 0 )
      {
         //initialize variables
         $tmp_array  = array();
         $sorted_set = array();
         $i          = 0;

         // create a temporary array of properties
         foreach ($this->result_set as $value)
         {
            $tmp_array[$i++] = $value->$property;
         }

         //sort the temporary array
         ($sort_type) ? arsort($tmp_array,$flag) : asort($tmp_array,$flag);

         //create new sorted set
         $i = 0;
         while (list ($k, $v) = each ($tmp_array))
            $sorted_set[$i++] = $this->result_set[$k];

         // replace the result_set with the sorted set
         $this->result_set = $sorted_set;
      }
   }



   // Class destructor
   function _DESTRUCTOR()
   {
      unset($this->result_set);
   }
}


// This function will clean the database fields and conmvert empty to null
function _Clean_Field($str)
{
   if (! isset($str))
   {
      $str = "NULL";
   } else {
      $str = mysql_escape_string($str);
      if (is_string($str)) $str = "'".($str)."'";
   }
   return $str;
} 

?>
