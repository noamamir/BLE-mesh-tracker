import {Injectable, OnDestroy} from '@angular/core';
import {Observable, ReplaySubject, share} from "rxjs";
import {HttpClient} from "@angular/common/http";
import * as io from 'socket.io-client'

@Injectable({
  providedIn: 'root'
})
export class SocketIoService implements OnDestroy {
  subscribersCounter: Record<string, number> = {};
  eventObservables$: Record<string, Observable<any>> = {};
  ioSocket: any;

  private isInitialized = new ReplaySubject<boolean>(1);
  public isInitialized$ = this.isInitialized.asObservable();

  constructor(private http: HttpClient) {
    this.connect()
  }



  ngOnDestroy() {
    this.disconnect()
  }

  of(namespace: string) {
    this.ioSocket.of(namespace);
  }

  on(eventName: string, callback: () => void) {
    this.ioSocket.on(eventName, callback)
  }

  once(eventName: string, callback: () => void) {
    this.ioSocket.once(eventName, callback)
  }

  private connect() {
    const options: any = {
      path: '/socket.io'
    }
    const ioFunc = (io as any).default ? (io as any).default : io;
    this.ioSocket = ioFunc(`ws://localhost:5000`, options);
    this.ioSocket.connect();
    this.isInitialized.next(true);
  }
  emit(eventName: string, data ?: any, callback ?: () => void) {
    this.ioSocket.disconnect.apply(this.ioSocket, arguments);
  }

  removeListener(eventName: string, callback: () => void) {
    return this.ioSocket.removeListener.apply(this.ioSocket, arguments)
  }

  disconnect(close ?: any) {
    this.ioSocket.disconnect.apply(this.ioSocket, arguments);
  }

  SubscribeToEvent<T>(eventName: string, callback: any) {
    if (!this.subscribersCounter[eventName]) {
      this.subscribersCounter[eventName] = 0;
    }
    this.subscribersCounter[eventName]++;

    if (!this.eventObservables$[eventName]) {
      this.eventObservables$[eventName] = new Observable((observer: any) => {
        const listener = (data: T) => {
          observer.next(data);
        };
        this.ioSocket.on(eventName, listener);
        return () => {
          this.subscribersCounter[eventName]--;
          if (this.subscribersCounter[eventName] === 0) {
            this.ioSocket.removeListener(eventName, listener);
            delete this.eventObservables$[eventName];
          }
        }
      }).pipe(share())
    }

    this.eventObservables$[eventName].subscribe(callback)
  }

}
