/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import { Reducer } from 'redux'
import { types } from '../constants/cryptoDotCom_types'

interface SupportedPair {
  base: string
  pair: string
  quote: string
  price: string
  quantity: string
}

function reducePairs (rawPairs: SupportedPair[]) {
  if (!rawPairs || !rawPairs.length) {
    return {}
  }
  return rawPairs.reduce((pairs: object, currPair: SupportedPair) => {
    const { base, pair } = currPair
    pairs[base] = pairs[base]
      ? [...pairs[base], pair]
      : [pair]
    return pairs
  }, {})
}

const cryptoDotComReducer: Reducer<NewTab.State | undefined> = (state: NewTab.State, action) => {
  const payload = action.payload

  switch (action.type) {
    case types.ON_BTC_PRICE_OPT_IN:
      state = { ...state }
      state.cryptoDotComState = {
        ...state.cryptoDotComState,
        optInBTCPrice: true,
        fetchStatus: 'pending'
      }
      break

    case types.ALL_ASSETS_DETAILS_REQUESTED:
      state = { ...state }
      state.cryptoDotComState = {
        ...state.cryptoDotComState,
        fetchStatus: 'pending'
      }
      break

    case types.ON_IS_CONNECTED_RECEIVED:
      state = { ...state }
      state.cryptoDotComState = {
        ...state.cryptoDotComState,
        isConnected: payload.isConnected
      }

      // Reset account specific data if disconnected.
      if (!payload.isConnected) {
        state.cryptoDotComState.newsEvents = []
        state.cryptoDotComState.depositAddresses = {}
        state.cryptoDotComState.accountBalances = {}
      }
      break;

    case types.SET_DISCONNECT_IN_PROGRESS:
      state = { ...state }
      state.cryptoDotComState = {
        ...state.cryptoDotComState,
        disconnectInProgress: payload.inProgress
      }
      break;
    case types.ALL_ASSETS_DETAILS_RECEIVED:
      state = { ...state }
      state.cryptoDotComState = {
        ...state.cryptoDotComState,
        fetchStatus: 'completed',
        charts: {
          ...state.cryptoDotComState.charts,
          ...payload.charts
        },
        tickerPrices: {
          ...state.cryptoDotComState.tickerPrices,
          ...payload.tickerPrices
        },
        depositAddresses: {
          ...state.cryptoDotComState.depositAddresses,
          ...payload.depositAddress
        }
      }
      break

    case types.ON_REFRESH_DATA:
      state = { ...state }
      state.cryptoDotComState = {
        ...state.cryptoDotComState,
        fetchStatus: 'refreshing'
      }
      break

    case types.HIDE_BALANCE:
      state = { ...state }
      state = {
        ...state,
        cryptoDotComState: {
          ...state.cryptoDotComState,
          hideBalance: payload.hide
        }
      }
      break

    case types.REFRESHED_DATA_RECEIVED:
      state = { ...state }
      state.cryptoDotComState = {
        ...state.cryptoDotComState,
        fetchStatus: 'completed',
        tickerPrices: {
          ...state.cryptoDotComState.tickerPrices,
          ...payload.tickerPrices
        },
        charts: payload.charts ? {
          ...state.cryptoDotComState.charts,
          ...payload.charts
        } : {},
        losersGainers: payload.losersGainers,
        accountBalances: payload.accountBalances ? payload.accountBalances : {},
        newsEvents: payload.newsEvents ? payload.newsEvents : [],
        supportedPairs: payload.pairs ? reducePairs(payload.pairs) : state.cryptoDotComState.supportedPairs,
        tradingPairs: payload.pairs ? payload.pairs : state.cryptoDotComState.tradingPairs
      }
      break

    default:
      break
  }

  return state
}

export default cryptoDotComReducer
