import { createStore, applyMiddleware, compose } from 'redux';
import thunk from 'redux-thunk';
import { hashHistory } from 'react-router';
import { syncHistory } from 'react-router-redux';
import rootReducer from '../reducers';

const router = syncHistory(hashHistory);
const enhancer = compose(
  applyMiddleware(thunk, router)
);

export default function configureStore(initialState) {
  const store = createStore(rootReducer, initialState, enhancer);

  router.listenForReplays(store);

  if (module.hot) {
    module.hot.accept('../reducers', () =>
      store.replaceReducer(require('../reducers'))
    );
  }

  return store;
}
