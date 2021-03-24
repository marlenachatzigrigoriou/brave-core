/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
import * as React from 'react'
import { ThemeProvider } from 'styled-components'
import Theme from 'brave-ui/theme/brave-default'

import { AssetViews } from './types'
import {
  BasicBox,
  CryptoDotComIcon,
  Header,
  Link,
  List,
  ListItem,
  PlainButton,
  StyledTitle,
  StyledTitleText,
  Text,
  WidgetWrapper,
  DisconnectWrapper,
  DisconnectTitle,
  DisconnectCopy,
  DisconnectButton,
  DismissAction
} from './style'

import { decimalizeCurrency } from './utils'

import AssetDepositView from './assetDepositView'
import AssetDetailView from './assetDetailView'
import AssetTradeView from './assetTradeView'
import BalanceSummaryView from './balanceSummaryView'
import PreOptInView from './preOptInView'
import TopMoversView from './topMoversView'
import TradeView from './tradeView'
import CryptoDotComLogo from './assets/cryptoDotCom-logo'

import createWidget from '../widget/index'
import { StyledTitleTab } from '../widgetTitleTab'
import { getLocale } from '../../../../common/locale'

enum MainViews {
  TOP,
  TRADE,
  EVENTS,
  BALANCE
}

interface State {
  currentView: MainViews
  currentAssetView: AssetViews
  selectedBase: string
  selectedQuote: string
  clientAuthUrl: string
}

// TODO(simonhong): Remove any from Props.
interface Props {
  showContent: boolean
  optInBTCPrice: boolean
  hideBalance: boolean
  isConnected: boolean
  disconnectInProgress: boolean
  accountBalances: Record<string, any>
  depositAddresses: Record<string, any>
  tickerPrices: Record<string, chrome.cryptoDotCom.TickerPrice>
  losersGainers: Record<string, chrome.cryptoDotCom.AssetRanking[]>
  supportedPairs: Record<string, string[]>
  tradingPairs: Array<Record<string, string>>
  newsEvents: Array<Record<string, string>>
  charts: Record<string, chrome.cryptoDotCom.ChartDataPoint[]>
  stackPosition: number
  onShowContent: () => void
  onDisableWidget: () => void
  onBtcPriceOptIn: () => void
  onUpdateActions: () => Promise<void>
  onAssetsDetailsRequested: (base: string, quote: string) => void
  onSetHideBalance: (hide: boolean) => void
  onIsConnected: (connected: boolean) => void
  disconnect: () => void
  cancelDisconnect: () => void
}

// TODO(simonhong): Don't cache account related infos in local storage.
// We should fetch every time when we refer to.
class CryptoDotCom extends React.PureComponent<Props, State> {
  private refreshDataInterval: any
  private checkConnectedStateInterval: any

  constructor (props: Props) {
    super(props)
    this.state = {
      currentView: MainViews.TOP,
      currentAssetView: AssetViews.DETAILS,
      selectedBase: '',
      selectedQuote: '',
      clientAuthUrl: '',
    }
  }

  componentDidUpdate(prevProps: Props) {
    if (prevProps.isConnected !== this.props.isConnected ||
        prevProps.optInBTCPrice !== this.props.optInBTCPrice ||
        prevProps.showContent !== this.props.showContent) {
      this.resetRefreshInterval()
    }
  }

  componentDidMount () {
    this.resetRefreshInterval()
    this.getClientURL()
  }

  componentWillUnmount () {
    this.clearIntervals()
  }

  resetRefreshInterval = () => {
    this.clearIntervals()

    // Stop background refresh if this is not visible.
    if (!this.props.showContent) {
      return
    }

    chrome.cryptoDotCom.isLoggedIn(async (loggedIn: boolean) => {
      if (loggedIn) {
        // Periodically check connect status if logged in.
        this.setCheckIsConnectedInterval()

        chrome.cryptoDotCom.isConnected(async (isConnected: boolean) => {
          this.props.onUpdateActions()
          this.checkSetRefreshInterval()
          this.props.onIsConnected(isConnected)
        })
      } else if (this.props.optInBTCPrice) {
        this.props.onUpdateActions()
        this.checkSetRefreshInterval()
        this.props.onIsConnected(false)
      }
    })
  }

  getClientURL = () => {
    chrome.cryptoDotCom.getClientUrl((clientAuthUrl: string) => {
      this.setState({ clientAuthUrl })
    })
  }

  setCheckIsConnectedInterval = () => {
    if (!this.checkConnectedStateInterval) {
      this.checkConnectedStateInterval = setInterval(() => {
        chrome.cryptoDotCom.isConnected((isConnected: boolean) => {
          this.props.onIsConnected(isConnected)
        })
      }, 30000)
    }
  }

  checkSetRefreshInterval = () => {
    if (!this.refreshDataInterval) {
      this.refreshDataInterval = setInterval(async () => {
        this.props.onUpdateActions()
          .catch((_e) => console.debug('Could not update crypto.com data'))
      }, 30000)
    }
  }

  clearIntervals = () => {
    clearInterval(this.refreshDataInterval)
    this.refreshDataInterval = null

    clearInterval(this.checkConnectedStateInterval)
    this.checkConnectedStateInterval = null
  }

  setMainView = (view: MainViews) => {
    this.setState({
      currentView: view
    })
  }

  clearAsset = () => {
    this.setState({
      selectedBase: '',
      selectedQuote: ''
    })
  }

  handleAssetClick = async (base: string, quote?: string, view?: AssetViews) => {
    this.setState({
      selectedBase: base,
      selectedQuote: quote || '',
      currentAssetView: view || AssetViews.DETAILS
    })
    this.props.onAssetsDetailsRequested(base, quote ? quote : 'USDT')
  }

  onClickConnectToCryptoDotCom = () => {
    window.open(this.state.clientAuthUrl, '_self', 'noopener')
  }

  renderTitle () {
    return (
      <Header showContent={this.props.showContent}>
        <StyledTitle>
          <CryptoDotComIcon>
            <CryptoDotComLogo />
          </CryptoDotComIcon>
          <StyledTitleText>
            {'Crypto.com'}
          </StyledTitleText>
        </StyledTitle>
      </Header>
    )
  }

  renderTitleTab () {
    const { onShowContent, stackPosition } = this.props

    return (
      <StyledTitleTab onClick={onShowContent} stackPosition={stackPosition}>
        {this.renderTitle()}
      </StyledTitleTab>
    )
  }

  renderNav () {
    const { currentView } = this.state
    return (
      <BasicBox isFlex={true} justify="start" $mb={13.5}>
        <PlainButton $pl="0" weight={500} textColor={currentView === MainViews.TOP ? 'white' : 'light'} onClick={this.setMainView.bind(null, MainViews.TOP)}>{getLocale('cryptoDotComWidgetTop')}</PlainButton>
        <PlainButton weight={500} textColor={currentView === MainViews.TRADE ? 'white' : 'light'} onClick={this.setMainView.bind(null, MainViews.TRADE)}>{getLocale('cryptoDotComWidgetTrade')}</PlainButton>
        <PlainButton weight={500} textColor={currentView === MainViews.EVENTS ? 'white' : 'light'} onClick={this.setMainView.bind(null, MainViews.EVENTS)}>{getLocale('cryptoDotComWidgetEvents')}</PlainButton>
        <PlainButton weight={500} textColor={currentView === MainViews.BALANCE ? 'white' : 'light'} onClick={this.setMainView.bind(null, MainViews.BALANCE)}>{getLocale('cryptoDotComWidgetBalance')}</PlainButton>
      </BasicBox>
    )
  }

  renderEvents () {
    return <List>
      {this.props.newsEvents.map((event: any) => (
        <ListItem $p={10} key={event.redirect_url}>
          <Text $fontSize={12} textColor='light'>{event.updated_at}</Text>
          <Text $fontSize={12}>{event.content}</Text>
          <Link $fontSize={12} $mt={5} inlineBlock={true} href={event.redirect_url} target='_blank'>{event.redirect_title}</Link>
        </ListItem>
      ))}
    </List>
  }

  renderCurrentView () {
    const { currentView } = this.state
    if (currentView === MainViews.TOP) {
      return <TopMoversView
        losersGainers={this.props.losersGainers}
        handleAssetClick={this.handleAssetClick}
      />
    }

    if (currentView === MainViews.TRADE) {
      return <TradeView
        tickerPrices={this.props.tickerPrices}
        losersGainers={this.props.losersGainers}
        tradingPairs={this.props.tradingPairs}
        handleAssetClick={this.handleAssetClick}
      />
    }

    if (currentView === MainViews.BALANCE) {
      return <BalanceSummaryView
        hideBalance={this.props.hideBalance}
        onSetHideBalance={this.props.onSetHideBalance}
        availableBalance={this.props.accountBalances.total_balance}
        holdings={this.props.accountBalances.accounts}
      />
    }

    if (currentView === MainViews.EVENTS) {
      return this.renderEvents()
    }

    return null;
  }

  getAvailableBalanceForCurrency = (currency: string) => {
    if (!this.props.accountBalances.accounts) {
      return 0
    }

    const account = this.props.accountBalances.accounts.find((account: Record<string, any>) => account.currency === currency)
    return decimalizeCurrency(account.available, account.currency_decimals)
  }

  renderAssetView () {
    const { currentAssetView, selectedBase, selectedQuote } = this.state
    const { tickerPrices, supportedPairs, charts, losersGainers} = this.props

    if (currentAssetView === AssetViews.DETAILS) {
      return <AssetDetailView
        currency={selectedBase}
        tickerPrice={tickerPrices[`${selectedBase}_USDT`]}
        pairs={supportedPairs[selectedBase] || []}
        chartData={charts[`${selectedBase}_USDT`] || []}
        losersGainers={losersGainers}
        handleAssetClick={this.handleAssetClick}
        handleBackClick={this.clearAsset}
      />
    }

    if (currentAssetView === AssetViews.TRADE) {
      const baseAvailable = this.getAvailableBalanceForCurrency(selectedBase)
      const quoteAvailable = this.getAvailableBalanceForCurrency(selectedQuote)
      const pair = this.props.tradingPairs.filter(pair => pair.base === selectedBase && pair.quote === selectedQuote)
      return <AssetTradeView
        tickerPrices={this.props.tickerPrices}
        availableBalanceBase={baseAvailable}
        availableBalanceQuote={quoteAvailable}
        base={selectedBase}
        quote={selectedQuote}
        priceDecimals={pair[0] ? pair[0].price : '0'}
        quantityDecimals={pair[0] ? pair[0].quantity : '0'}
        handleBackClick={this.clearAsset}
      />
    }

    if (currentAssetView === AssetViews.DEPOSIT) {
      const depositAddress = this.props.depositAddresses[selectedBase]
      return <AssetDepositView
        assetAddress={depositAddress ? depositAddress.address : ''}
        assetQR={depositAddress ? depositAddress.qr_code : ''}
        base={selectedBase}
        handleBackClick={this.clearAsset}
      />
    }

    return null
  }

  renderIndex () {
    const { selectedBase } = this.state

    if (selectedBase) {
      return this.renderAssetView()
    } else {
      return <>
        {this.renderNav()}
        {this.renderCurrentView()}
      </>
    }
  }

  renderDisconnectView = () => {
    return (
      <DisconnectWrapper>
        <DisconnectTitle>
          {getLocale('cryptoDotComWidgetDisconnectTitle')}
        </DisconnectTitle>
        <DisconnectCopy>
          {getLocale('cryptoDotComWidgetDisconnectText')}
        </DisconnectCopy>
        <DisconnectButton onClick={this.props.disconnect}>
          {getLocale('cryptoDotComWidgetDisconnectButton')}
        </DisconnectButton>
        <DismissAction onClick={this.props.cancelDisconnect}>
          {getLocale('cryptoDotComWidgetCancelText')}
        </DismissAction>
      </DisconnectWrapper>
    )
  }

  render () {
    if (!this.props.showContent) {
      return this.renderTitleTab()
    }

    const extendedTheme = {
      ...Theme,
      palette: {
        ...Theme.palette,
        primary: 'rgba(68, 176, 255, 1)',
        secondary: 'rgba(15, 28, 45, 0.7)'
      }
    }

    return (
      <ThemeProvider theme={extendedTheme}>
        <WidgetWrapper>
          {
            this.props.disconnectInProgress
            ? this.renderDisconnectView()
            : <>
                {this.renderTitle()}
                {(this.props.isConnected) ? (
                  this.renderIndex()
                ) : (
                  <PreOptInView
                    optInBTCPrice={this.props.optInBTCPrice}
                    losersGainers={this.props.losersGainers || {}}
                    tickerPrices={this.props.tickerPrices}
                    onBTCPriceOptedIn={this.props.onBtcPriceOptIn}
                  />
                )}
              </>
          }
        </WidgetWrapper>
      </ThemeProvider>
    )
  }
}

export const CryptoDotComWidget = createWidget(CryptoDotCom)
