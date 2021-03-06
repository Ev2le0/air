/*
  +----------------------------------------------------------------------+
  | air framework                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) wukezhan<wukezhan@gmail.com>                           |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: wukezhan<wukezhan@gmail.com>                                 |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "Zend/zend_interfaces.h"

#include "php_air.h"

#include "src/air_async_waiter.h"
#include "src/air_async_service.h"

zend_class_entry *air_async_service_ce;

/* {{{ ARG_INFO */
ZEND_BEGIN_ARG_INFO_EX(air_async_service_construct_arginfo, 0, 0, 2)
	ZEND_ARG_INFO(0, waiter)
	ZEND_ARG_INFO(0, request)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_async_service_get_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ PHP METHODS */
PHP_METHOD(air_async_service, __construct) {
	AIR_INIT_THIS;

	zval *waiter = NULL;
	zval *request = NULL;
	if(zend_parse_parameters(ZEND_NUM_ARGS(), "zz", &waiter, &request) == FAILURE){
		AIR_NEW_EXCEPTION(1, "invalid __construct params");
	}
	//check if waiter is instance of air\waiter
	if(Z_TYPE_P(waiter) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(waiter), air_async_waiter_ce)) {
		air_throw_exception(1, "param waiter must be a instance of air\\waiter");
	}
	zend_update_property(air_async_service_ce, self, ZEND_STRL("_waiter"), waiter);
	zend_update_property(air_async_service_ce, self, ZEND_STRL("_request"), request);
	zval *__id = zend_read_static_property(air_async_service_ce, ZEND_STRL("__id"), 0);
	Z_LVAL_P(__id)++;
	zend_update_property_long(air_async_service_ce, self, ZEND_STRL("_id"), Z_LVAL_P(__id));
}

//call the service
PHP_METHOD(air_async_service, call) {
	AIR_INIT_THIS;

	zval *waiter = zend_read_property(air_async_service_ce, self, ZEND_STRL("_waiter"), 0, NULL);
	zval *self_id = zend_read_property(air_async_service_ce, self, ZEND_STRL("_id"), 0, NULL);
	zval ret;
	zval params[1] = {*self_id};
	air_call_object_method(waiter, Z_OBJCE_P(waiter), "response", &ret, 1, params);
	RETURN_ZVAL(&ret, 1, 1);
}

PHP_METHOD(air_async_service, __get) {
	AIR_INIT_THIS;

	zend_string *key;
	if(zend_parse_parameters(ZEND_NUM_ARGS(), "S", &key) == FAILURE){
		AIR_NEW_EXCEPTION(1, "invalid __get params");
	}

	zval *ret = NULL;
	if(ZSTR_LEN(key)==sizeof("id") && !strcmp(ZSTR_VAL(key), "id")){
		ret = zend_read_property(air_async_service_ce, self, ZEND_STRL("_id"), 0, NULL);
	}else if(ZSTR_LEN(key)==sizeof("request") && !strcmp(ZSTR_VAL(key), "request")){
		ret = zend_read_property(air_async_service_ce, self, ZEND_STRL("_request"), 0, NULL);
	}else if(ZSTR_LEN(key)==sizeof("response") && !strcmp(ZSTR_VAL(key), "response")){
		zval *waiter = zend_read_property(air_async_service_ce, self, ZEND_STRL("_waiter"), 0, NULL);
		zval *self_id = zend_read_property(air_async_service_ce, self, ZEND_STRL("_id"), 0, NULL);
		zend_call_method_with_1_params(waiter, Z_OBJCE_P(waiter), NULL, "response", return_value, self_id);
	}

	if(ret){
		RETURN_ZVAL(ret, 1, 0);
	}
}
/* }}} */

/* {{{ air_async_service_methods */
zend_function_entry air_async_service_methods[] = {
	PHP_ME(air_async_service, __construct, air_async_service_construct_arginfo,  ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(air_async_service, __get, air_async_service_get_arginfo,  ZEND_ACC_PUBLIC)
	PHP_ME(air_async_service, call, NULL,  ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ AIR_MINIT_FUNCTION */
AIR_MINIT_FUNCTION(air_async_service) {
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "air\\async\\service", air_async_service_methods);

	air_async_service_ce = zend_register_internal_class_ex(&ce, NULL);
	air_async_service_ce->ce_flags |= ZEND_ACC_FINAL;

	zend_declare_property_long(air_async_service_ce, ZEND_STRL("__id"), 0, ZEND_ACC_PROTECTED | ZEND_ACC_STATIC);
	zend_declare_property_null(air_async_service_ce, ZEND_STRL("_id"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(air_async_service_ce, ZEND_STRL("_waiter"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(air_async_service_ce, ZEND_STRL("_request"), ZEND_ACC_PROTECTED);
	return SUCCESS;
}
/* }}} */

