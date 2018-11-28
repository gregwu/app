<?php
include ".config.php";    
class DBConnection {
    private static $instance;
    
    public $connection;    
    public $database;
    
    private function __construct()
    {
        $connectionString = sprintf('mongodb://%s:%s@%s:%d/%s', USERNAME,PASSWORD, HOST, PORT, DBNAME );
        try {
            
            $this->manager = new MongoDB\Driver\Manager($connectionString);
            $this->database = DBNAME;
            $this->writeConcern = new MongoDB\Driver\WriteConcern(MongoDB\Driver\WriteConcern::MAJORITY, 1000);
        
        } catch (MongoConnectionException $e) {
            throw $e;
        }
    }
    
    static public function instantiate()
    {
        if (!isset(self::$instance)) {
            $class = __CLASS__;
            self::$instance = new $class;
        }
        
        return self::$instance;
    }

	//reading
	public function query($collection, $filter=[], $options = [])
	{
		try {
		$this->query = new MongoDB\Driver\Query($filter, $options);	
//		$this->readPreference = new MongoDB\Driver\ReadPreference(MongoDB\Driver\ReadPreference::RP_PRIMARY);
		return $this->manager->executeQuery($this->database.'.'.$collection, $this->query);//, $this->readPreference);
/*
foreach($cursor as $document) {
    var_dump($document);
}
*/
		} catch(MongoDB\Driver\Exception $e) {
                    echo $e->getMessage(), "\n";
                    exit;
                }
	}

	public function findone($collection, $filter=[], $options = [])
	{
		$cursor = $this->query($collection, $filter, $options);
		$a = $cursor->toArray();
		if (isset($a[0]) ) 
			return $a[0];

		return null;
	}

	//saving
	public function command($command)
	{
		try{
			$cursor = $this->manager->executeCommand($this->database, new MongoDB\Driver\Command($command));
			return $cursor;

//			$response = $cursor->toArray()[0];
//			return $esponse;
		} catch(MongoDB\Driver\Exception $e) {
		    echo $e->getMessage(), "\n";
		    exit;
		}
	}
    

	public function insert($collection, $data)
	{
		$bulk = new MongoDB\Driver\BulkWrite();
		$id = $bulk->insert($data);
		$result = $this->manager->executeBulkWrite($this->database.'.'.$collection, $bulk, $this->writeConcern);
		return $id->__toString();
	}

	public function update($collection, $filter, $data, $option=[])
        {
                $bulk = new MongoDB\Driver\BulkWrite();
                $bulk->update($filter, $data, $option);
		$result = $this->manager->executeBulkWrite($this->database.'.'.$collection, $bulk, $this->writeConcern);
		return $result->getModifiedCount();
        }

        public function delete($collection, $filter, $option=array())
        {
                $bulk = new MongoDB\Driver\BulkWrite();
                $bulk->delete( $filter, $option);
		$result = $this->manager->executeBulkWrite($this->database.'.'.$collection, $bulk, $this->writeConcern);
		return $result->getDeletedCount();
        }
}
