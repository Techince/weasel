#! /usr/bin/env lua
--
-- rime.lua
-- Copyright (C) 2019 tiglog <me@tiglog.com>
--
-- Distributed under terms of the MIT license.
--
datetime_translator = require("datetime")
calculator_translator = require("calculator")
auto_select_phrase_processor = require("auto_select_phrase")
start_translator = require("start")

-- 数字、人民币大写，R 开头
number_translator = require("number_translator")
