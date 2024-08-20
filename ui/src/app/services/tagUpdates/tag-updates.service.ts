import {Injectable, NgZone} from '@angular/core';
import {TagMessage} from "../../models/TagMessage";
import {interval, map, Subject} from "rxjs";
import {SocketIoService} from "../socket-io.service";

@Injectable({
  providedIn: 'root'
})
export class TagUpdatesService {

  updates = new Subject<TagMessage>()

  constructor(private ngZone: NgZone, private socketIo: SocketIoService) {
    this.socketIo.SubscribeToEvent<TagMessage>('tag_message', (data: TagMessage) => this.updates.next(data));
  }


}
