CREATE TABLE `url_map` (
 `id` bigint NOT NULL AUTO_INCREMENT,
 `short_key` varchar(32) NULL ,
 `original_url` varchar(512) NULL ,
 `times` int NOT NULL  DEFAULT '0',
 `create_at` bigint NOT NULL DEFAULT '0',
 `update_at` bigint NOT NULL DEFAULT '0',
 PRIMARY KEY (`id`),
 unique (`original_url`),
 KEY `index_update_at` (`update_at` DESC),
 KEY `index_short_key` (`short_key`),
 KEY `index_original_url` (`original_url`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

CREATE TABLE `url_map_user` (
 `id` bigint NOT NULL AUTO_INCREMENT,
 `short_key` varchar(32) NULL ,
 `original_url` varchar(512) NULL ,
 `times` int NOT NULL  DEFAULT '0',
 `create_at` bigint NOT NULL DEFAULT '0',
 `update_at` bigint NOT NULL DEFAULT '0',
 PRIMARY KEY (`id`),
 unique (`original_url`),
 KEY `index_update_at` (`update_at` DESC),
 KEY `index_short_key` (`short_key`),
 KEY `index_original_url` (`original_url`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;