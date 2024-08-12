import {Injectable} from '@angular/core';
import {NgxIndexedDBService, WithID} from "ngx-indexed-db";
import {ReceiverId} from "../../models/receiver";
import {BehaviorSubject, Observable} from "rxjs";

@Injectable({
  providedIn: 'root'
})
export class ReceiverAliasService {
  aliasMap = new BehaviorSubject<Readonly<Record<ReceiverId, string>>>({})

  constructor() {
    this.setAlias('rec003', 'engine room')
    this.setAlias('rec001', 'upper deck')
  }

  setAlias(receiver: ReceiverId, alias: string) {
    const entry: Record<ReceiverId, string> = {}
    entry[receiver] = alias

    this.aliasMap.next({...this.aliasMap.value, ...entry})
  }
}
