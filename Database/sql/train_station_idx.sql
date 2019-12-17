-- MySQL dump 10.13  Distrib 5.7.28, for Linux (x86_64)
--
-- Host: localhost    Database: train
-- ------------------------------------------------------
-- Server version	5.7.28-0ubuntu0.16.04.2-log

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `station_idx`
--

DROP TABLE IF EXISTS `station_idx`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `station_idx` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `train` varchar(45) COLLATE utf8_bin NOT NULL,
  `station` varchar(45) COLLATE utf8_bin NOT NULL,
  `idx` int(10) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  KEY `index2` (`train`,`station`)
) ENGINE=InnoDB AUTO_INCREMENT=49 DEFAULT CHARSET=utf8 COLLATE=utf8_bin;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `station_idx`
--

LOCK TABLES `station_idx` WRITE;
/*!40000 ALTER TABLE `station_idx` DISABLE KEYS */;
INSERT INTO `station_idx` VALUES (7,'G102','上海虹桥',1),(8,'G102','无锡东',2),(9,'G102','常州北',3),(10,'G102','南京南',4),(11,'G102','滁州',5),(12,'G102','蚌埠南',6),(13,'G102','宿州东',7),(14,'G102','徐州东',8),(15,'G102','济南西',9),(16,'G102','廊坊',10),(17,'G102','北京南',11),(23,'G6','上海',1),(24,'G6','苏州北',2),(25,'G6','南京南',3),(26,'G6','济南西',4),(27,'G6','北京南',5),(28,'Z282','杭州',1),(29,'Z282','海宁',2),(30,'Z282','嘉兴',3),(31,'Z282','上海南',4),(32,'Z282','苏州',5),(33,'Z282','无锡',6),(34,'Z282','常州',7),(35,'Z282','镇江',8),(36,'Z282','南京',9),(37,'Z282','蚌埠',10),(38,'Z282','徐州',11),(39,'Z282','兖州',12),(40,'Z282','德州',13),(41,'Z282','天津西',14),(42,'Z282','北京',15),(43,'Z282','张家口南',16),(44,'Z282','大同',17),(45,'Z282','集宁南',18),(46,'Z282','呼和浩特东',19),(47,'Z282','包头东',20),(48,'Z282','包头',21);
/*!40000 ALTER TABLE `station_idx` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2019-12-17  1:29:29
