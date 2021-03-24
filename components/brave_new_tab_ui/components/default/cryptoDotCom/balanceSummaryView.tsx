// Copyright (c) 2021 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

import * as React from 'react'

import {
  Balance,
  BasicBox,
  BlurIcon,
  FlexItem,
  List,
  ListItem,
  Text
} from './style'

import {
  formattedNum,
  decimalizeCurrency,
  renderIconAsset
} from './utils'

import {
  ShowIcon,
  HideIcon
} from '../../default/exchangeWidget/shared-assets'

import { getLocale } from '../../../../common/locale'

// TODO(simonhong): make this as global type such as chrome.cryptoDotCom.accountBalances.account
interface Account {
  currency: string
  available: string
  currency_decimals: number
}

interface Props {
  onSetHideBalance: (hide: boolean) => void
  hideBalance: boolean
  availableBalance: string
  holdings: Account[]
}

export default function BalanceSummaryView ({
  onSetHideBalance,
  hideBalance,
  availableBalance,
  holdings
}: Props) {
  // Only shows non-empty holdings.
  holdings = holdings ? holdings.filter(item => item.available !== '0') : []

  return <>
    <BasicBox isFlex={true} $mb={18}>
      <FlexItem>
        <Text textColor='light' $mb={5} $fontSize={12}>{getLocale('cryptoDotComWidgetAvailableBalance')}</Text>
        <Balance hideBalance={hideBalance}>
          <Text lineHeight={1.15} $fontSize={21}>{formattedNum(Number(availableBalance))}</Text>
        </Balance>
      </FlexItem>
      <FlexItem>
        <BlurIcon onClick={() => onSetHideBalance(!hideBalance)}>
          {
            hideBalance
            ? <ShowIcon />
            : <HideIcon />
          }
        </BlurIcon>
      </FlexItem>
    </BasicBox>
    <Text textColor='light' $mb={5} $fontSize={12}>{getLocale('cryptoDotComWidgetHoldings')}</Text>
    <List>
      {holdings.map(({currency, available, currency_decimals}) => {
        return (
          <ListItem key={currency} isFlex={true} $height={40}>
            <FlexItem $pl={5} $pr={5}>
              {renderIconAsset(currency.toLowerCase())}
            </FlexItem>
            <FlexItem>
              <Text>{currency}</Text>
            </FlexItem>
            <FlexItem textAlign='right' flex={1}>
              <Balance hideBalance={hideBalance}>
                {(available !== null) && <Text lineHeight={1.15}>{decimalizeCurrency(available, currency_decimals)}</Text>}
              </Balance>
            </FlexItem>
          </ListItem>
        )
      })}
    </List>
  </>
}
