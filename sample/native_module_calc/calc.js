/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

const calc = requireInternal('calc');

const number = requireNapi('number');

function sub(x, y)
{
    return x - y;
}

export default {
    add: calc.add,
    sub: sub,
    ValueConstant: {
        TYPE_VALUE_0: 0,
        TYPE_VALUE_1: number.Number.ONE,
    }
}