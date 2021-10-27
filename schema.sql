/* Database: api */
CREATE TABLE IF NOT EXISTS `inc` (
	`key` INT(11) NOT NULL,
	`count` INT(11) NOT NULL DEFAULT 0,
	UNIQUE KEY `key` (`key`)
) DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `pairs` (
	`key` VARCHAR(50) NOT NULL,
	`value` TEXT NOT NULL,
	UNIQUE KEY `key` (`key`)
) DEFAULT CHARSET=utf8mb4;

/* Database: api-log */
CREATE TABLE IF NOT EXISTS `log` (
	`id` INT(11) NOT NULL AUTO_INCREMENT,
	`info` TEXT NOT NULL,
	`ip` VARCHAR(50) NOT NULL,
	`timestamp` INT(10) UNSIGNED NOT NULL,
	PRIMARY KEY (`id`)
) DEFAULT CHARSET=utf8mb4;
